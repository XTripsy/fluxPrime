#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Generics/Systems/Ability/FluxPrimeBaseAbilitySystem.h"
#include "StructUtils/InstancedStruct.h"
#include "Domains/Crowds/Systems/Ability/FluxPrimeCrowdsMeleAbility.h"
#include "Domains/Crowds/Systems/Ability/FluxPrimeCrowdsProjectileAbility.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperBool.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeCrowdsAbilitySystem.generated.h"

USTRUCT(BlueprintType)
struct FAbilitySystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRotationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRequestAbilityTag;

	bool IsValid() const
	{
		return EntityDataLocationTag.IsValid() 
		   && EntityDataRotationTag.IsValid()
		   && EntityDataRequestAbilityTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsAbilitySystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsNiagaraCallback> niagaraCallback = nullptr;
	
	TArray<FGameplayTag>* catalogAbilityFragmentIdentity = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
	TArray<float> damageMapping;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsAbilitySystem : public FFluxPrimeBaseAbilitySystem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FAbilitySystemDataTagConfig DataTagConfig;
	
	TArray<float> DamageMapping;
	TArray<int16> EntityDataLocationID, EntityDataRotationID, EntityDataTypeID, EntityDataRequestAbilityID;
	
private:
	void RegisterAbilityExecutors(const FFluxPrimeCrowdsAbilitySystemsContext& context)
	{
		int32 count = context.catalogAbilityFragmentIdentity->Num();
		
		AbilityExecutors.Reset();
		AbilityExecutors.Reserve(count);
		
		for (int i = 0; i < count; ++i)
		{
			FGameplayTag catalogIdentity = (*context.catalogAbilityFragmentIdentity)[i];
			
			int32 index = AbilityFragments.IndexOfByPredicate(
			[&](const FInstancedStruct& fragment)
				{
					if (const FFluxPrimeCrowdsMeleAbility* Mele = fragment.GetPtr<FFluxPrimeCrowdsMeleAbility>())
						return Mele->Identity == catalogIdentity;

					if (const FFluxPrimeCrowdsProjectileAbility* Projectile = fragment.GetPtr<FFluxPrimeCrowdsProjectileAbility>())
						return Projectile->Identity == catalogIdentity;

					return false;
				}
			);

			if (index == INDEX_NONE)
			{
				RegisterEmptyAbility();
				continue;
			}
			
			if (AbilityFragments[index].GetPtr<FFluxPrimeCrowdsMeleAbility>() &&
				AbilityFragments[index].GetPtr<FFluxPrimeCrowdsMeleAbility>()->Identity == catalogIdentity)
			{
				const int32 executorIndex = RegisterAbility<FFluxPrimeCrowdsMeleAbility>(AbilityFragments[index]);
				
				FFluxPrimeCrowdsMeleAbilitySystemsContext tempContext;
				tempContext.world = context.world;

				FAbilityExecutor& Executor = AbilityExecutors[executorIndex];
				FFluxPrimeCrowdsMeleAbility& ability = Executor.Data->GetMutable<FFluxPrimeCrowdsMeleAbility>();
				ability.InitializedMeleAbilitySystems(tempContext);
				continue;
			}
			
			if (AbilityFragments[index].GetPtr<FFluxPrimeCrowdsProjectileAbility>() &&
				AbilityFragments[index].GetPtr<FFluxPrimeCrowdsProjectileAbility>()->Identity == catalogIdentity)
			{
				const int32 executorIndex = RegisterAbility<FFluxPrimeCrowdsProjectileAbility>(AbilityFragments[index]);
				
				FFluxPrimeCrowdsProjectileAbilitySystemsContext tempContext;
				tempContext.world = context.world;
				tempContext.niagaraCallback = context.niagaraCallback;
				
				FAbilityExecutor& Executor = AbilityExecutors[executorIndex];
				FFluxPrimeCrowdsProjectileAbility& ability = Executor.Data->GetMutable<FFluxPrimeCrowdsProjectileAbility>();
				ability.InitializedProjectileAbilitySystems(tempContext);
				continue;
			}
			
			RegisterEmptyAbility();
		}
	}
	
public:
	void Initialized(FFluxPrimeCrowdsAbilitySystemsContext context)
	{
		check(context.world);
		check(context.catalogAbilityFragmentIdentity);
		check(context.niagaraCallback);
		check(context.dataStores);

		DamageMapping = context.damageMapping;
		
		for (auto& dataStore : *context.dataStores)
		{
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataRotationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRotationTag));
			EntityDataRequestAbilityID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestAbilityTag));
		}
		
		if (DamageMapping.IsEmpty()) DamageMapping.Init(0, context.dataStores->Num());
		
		RegisterAbilityExecutors(context);
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Ability_Systems);
		
		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
			FRotator* entityDataRotation = reinterpret_cast<FRotator*>(dataStore.GetRawBufferData(EntityDataRotationID[indexDataID]));
			FFluxPrimeWrapperBool* entityDataRequestAbility = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataRequestAbilityID[indexDataID]));
			
			check(entityDataLocation);
			check(entityDataRotation);
			check(entityDataRequestAbility);
			
			FRotator rotationEntity = FRotator::ZeroRotator;
			
			for (int i = 0; i < dataStore.GetActiveEntityCount(); ++i)
			{
				if (!entityDataRequestAbility[i].ValueBool) continue;
				
				entityDataRequestAbility[i].ValueBool = false;
				
				check(indexDataID < AbilityExecutors.Num());
				
				FAbilityExecutor& executor = AbilityExecutors[indexDataID];

				check(executor.Execute != nullptr);

				rotationEntity = FRotator(0, entityDataRotation[i].Yaw + 85.0f, 0);
				executor.Execute(
					*executor.Data,
					entityDataLocation[i],
					rotationEntity,
					DamageMapping[indexDataID]
				);
			}
			
			indexDataID++;
		}
	}
};
