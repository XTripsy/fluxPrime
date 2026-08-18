#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystem.h"
#include "StructUtils/InstancedStruct.h"
#include "AbilitySystems/FluxPrimeMeleAbilitySystem.h"
#include "AbilitySystems/FluxPrimeProjectileAbilitySystem.h"
#include "FluxPrimeAbilitySystem.generated.h"

using FAbilityExecute = void(*)(FInstancedStruct&, FVector, float, float);

struct FAbilityExecutor
{
	FInstancedStruct Data;
	FAbilityExecute Execute = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeAbilitySystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeNiagaraCallback> niagaraCallback = nullptr;
	
	TArray<TObjectPtr<UScriptStruct>>* crowdsCatalogFragment = nullptr;
	TArray<FGuid>* crowdsCatalogFragmentID = nullptr;
	uint16 crowdsCountCatalog; 
	TArray<FVector>* locationCrowds = nullptr;
	TArray<float>* rotationCrowds = nullptr;
	TArray<float>* damageCrowds = nullptr;
	TArray<bool>* requestAbilityCrowds = nullptr;
	TArray<int8>* typeCrowds = nullptr;
	TArray<FInstancedStruct> abilityFragments;
	uint16* memberActive = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeAbilitySystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
	TArray<FVector>* LocationCrowds = nullptr;
	TArray<float>* RotationCrowds = nullptr;
	TArray<float>* DamageCrowds = nullptr;
	TArray<bool>* RequestAbilityCrowds = nullptr;
	TArray<int8>* CrowdsType = nullptr;
	uint16* MemberActive = nullptr;
	
	TArray<FAbilityExecutor> AbilityExecutors;
	
private:
	template<typename T>
	int32 RegisterAbility(const FInstancedStruct& source)
	{
		FAbilityExecutor executor;
		executor.Data.InitializeAs<T>();
		executor.Data.GetMutable<T>() = source.Get<T>();
		executor.Execute = &ExecuteAbility<T>;

		return AbilityExecutors.Add(MoveTemp(executor));
	}
	
	int32 RegisterEmptyAbility()
	{
		FAbilityExecutor Executor;
		Executor.Execute = &ExecuteEmptyAbility;

		return AbilityExecutors.Add(MoveTemp(Executor));
	}
	
	template<typename T>
	static void ExecuteAbility(FInstancedStruct& data, FVector location, float rotation, float damage)
	{
		T& ability = data.GetMutable<T>();

		ability.Execute(location, rotation, damage);
	}
	
	static void ExecuteEmptyAbility(FInstancedStruct& Data, FVector Location, float Rotation, float Damage)
	{
		UE_LOG(LogTemp, Log, TEXT("ABILITY SYSTEMS:: EMPTY ABILITY"));
	}
	
	void RegisterAbilityExecutors(const uint16& crowdsCountCatalog, const TArray<FInstancedStruct>& abilityFragments, 
		const TArray<TObjectPtr<UScriptStruct>>& crowdsCatalogFragment, const TArray<FGuid>& crowdsCatalogFragmentID, const TObjectPtr<UFluxPrimeNiagaraCallback> niagaraCallback)
	{
		AbilityExecutors.Reset();
		AbilityExecutors.Reserve(crowdsCountCatalog);
		
		for (int i = 0; i < crowdsCountCatalog; ++i)
		{
			TObjectPtr<UScriptStruct> abilityFragment = crowdsCatalogFragment[i];
			FGuid abilityID = crowdsCatalogFragmentID[i];
			
			int32 index = abilityFragments.IndexOfByPredicate(
			[&](const FInstancedStruct& fragment)
				{
					if (fragment.GetScriptStruct() != abilityFragment)
						return false;

					if (const FFluxPrimeMeleAbilitySystem* Mele = fragment.GetPtr<FFluxPrimeMeleAbilitySystem>())
						return Mele->ID == abilityID;

					if (const FFluxPrimeProjectileAbilitySystem* Projectile = fragment.GetPtr<FFluxPrimeProjectileAbilitySystem>())
						return Projectile->ID == abilityID;

					return false;
				}
			);

			if (index == INDEX_NONE)
			{
				RegisterEmptyAbility();
				continue;
			}
			
			if (abilityFragments[index].GetPtr<FFluxPrimeMeleAbilitySystem>() &&
				abilityFragments[index].GetPtr<FFluxPrimeMeleAbilitySystem>()->ID == abilityID &&
				abilityFragment == FFluxPrimeMeleAbilitySystem::StaticStruct())
			{
				const int32 executorIndex = RegisterAbility<FFluxPrimeMeleAbilitySystem>(abilityFragments[index]);
				
				FFluxPrimeMeleAbilitySystemsContext context;
				context.world = World;

				FAbilityExecutor& Executor = AbilityExecutors[executorIndex];
				FFluxPrimeMeleAbilitySystem& ability = Executor.Data.GetMutable<FFluxPrimeMeleAbilitySystem>();
				ability.InitializedMeleAbilitySystems(context);
				continue;
			}
			
			if (abilityFragments[index].GetPtr<FFluxPrimeProjectileAbilitySystem>() &&
				abilityFragments[index].GetPtr<FFluxPrimeProjectileAbilitySystem>()->ID == abilityID &&
				abilityFragment == FFluxPrimeProjectileAbilitySystem::StaticStruct())
			{
				const int32 executorIndex = RegisterAbility<FFluxPrimeProjectileAbilitySystem>(abilityFragments[index]);
				
				FFluxPrimeProjectileAbilitySystemsContext context;
				context.world = World;
				context.niagaraCallback = niagaraCallback;
				
				FAbilityExecutor& Executor = AbilityExecutors[executorIndex];
				FFluxPrimeProjectileAbilitySystem& ability = Executor.Data.GetMutable<FFluxPrimeProjectileAbilitySystem>();
				ability.InitializedProjectileAbilitySystems(context);
				continue;
			}
			
			RegisterEmptyAbility();
		}
	}
	
public:
	void InitializedAbilitySystems(FFluxPrimeAbilitySystemsContext context)
	{
		check(context.world);
		check(context.crowdsCatalogFragment);
		check(context.crowdsCatalogFragmentID);
		check(context.niagaraCallback);
		check(context.locationCrowds);
		check(context.rotationCrowds);
		check(context.damageCrowds);
		check(context.requestAbilityCrowds);
		check(context.typeCrowds);
		check(context.memberActive);
		
		World = context.world;
		LocationCrowds = context.locationCrowds;
		RotationCrowds = context.rotationCrowds;
		DamageCrowds = context.damageCrowds;
		RequestAbilityCrowds = context.requestAbilityCrowds;
		CrowdsType = context.typeCrowds;
		MemberActive = context.memberActive;
		
		RegisterAbilityExecutors(context.crowdsCountCatalog, context.abilityFragments, 
			*context.crowdsCatalogFragment, *context.crowdsCatalogFragmentID, context.niagaraCallback);
	}
	
	void UpdateAbilitySystems()
	{
		auto& locationCrowds = *LocationCrowds;
		auto& rotationCrowds = *RotationCrowds;
		auto& damageCrowds = *DamageCrowds;
		auto& typeCrowds = *CrowdsType;
		auto& abilityCrowds = *RequestAbilityCrowds;
		
		for (int i = 0; i < *MemberActive; ++i)
		{
			if (!abilityCrowds[i]) continue;
			abilityCrowds[i] = false;
			
			check(typeCrowds[i] >= 0);
			check(typeCrowds[i] < AbilityExecutors.Num());
			
			FAbilityExecutor& executor = AbilityExecutors[typeCrowds[i]];

			check(executor.Execute != nullptr);

			executor.Execute(
				executor.Data,
				locationCrowds[i],
				rotationCrowds[i],
				damageCrowds[i]
			);
		}
	}
};
