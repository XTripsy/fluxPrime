#pragma once

#include "CoreMinimal.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsData.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Domains/Crowds/Configurations/SystemsConfiguration.h"
#include "Framework/Core/FluxPrimeBaseModule.h"
#include "Generics/Systems/FluxPrimeBoidsSystem.h"
#include "Generics/Systems/FluxPrimeGroundHeightSystem.h"
#include "Generics/Systems/FluxPrimeMovementSystem.h"
#include "Generics/Systems/Navigation/FluxPrimeNavigationSystem.h"
#include "Generics/Systems/Animation/FluxPrimeAnimationSystem.h"
#include "Generics/Systems/FluxPrimeRenderSystem.h"
#include "Generics/Systems/FluxPrimeHealthSystem.h"
#include "Domains/Crowds/Systems/FluxPrimeCrowdsCompactSystem.h"
#include "Domains/Crowds/Systems/Ability/FluxPrimeCrowdsAbilitySystem.h"
#include "Domains/Crowds/Systems/Notify/FluxPrimeCrwodsAnimationNotifySystem.h"
#include "Domains/Crowds/Systems/State/FluxPrimeCrowdsStateMachineSystem.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsNiagaraCallback.h"
#include "Domains/Crowds/Systems/FluxPrimeCrowdsTargetSystem.h"
#include "Generics/Systems/FluxPrimeRaycastSystem.h"
#include "Domains/Crowds/Systems/FluxPrimeCrowdsSnapshotSystem.h"
#include "Generics/Systems/FluxPrimeVisualInterpolationSystem.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeCrowdsSystemsModule.generated.h"

USTRUCT()
struct FFluxPrimeCrowdsSystemsModuleContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* crowdsComponents = nullptr;
	TArray<FFluxPrimeArchetypeEntityCatalog>* entityCatalog = nullptr;
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>>* crowdsAnimationSoftRef = nullptr;
	
	FOnCrowdsManagerActionChange* onCrowdsManagerActionChange = nullptr;
	TArray<FFluxPrimeTargetCatalog>* crowdsTarget = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
	
	bool hasAuthority = false;
	
	FGetStateID getStateID = nullptr;
	FGetNotifyID getNotifyID = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsSystemsModule : public FFluxPrimeBaseModule
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere, meta = (BaseStruct="FluxPrimeBaseSystem", ExcludeBaseStruct, AllowPrivateAccess = true))
	TObjectPtr<USystemsConfiguration> SystemsConfiguration;
	
	UPROPERTY()
	TArray<FInstancedStruct> SystemFragments;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsNiagaraCallback> NiagaraCallback = nullptr;
	
	int32 CrowdsAnimationSystemIndex = INDEX_NONE, CrowdsHealthSystemIndex = INDEX_NONE, CrowdsTargetSystemIndex = INDEX_NONE, CrowdsNavigationSystemIndex = INDEX_NONE, CrowdsRaycastSystemIndex = INDEX_NONE, CrowdsSnapshotSystemIndex = INDEX_NONE;
	
private:
	template <typename T>
	bool CanRunInCurrentAuthority(const T* SystemConfig, bool bIsAuthority) 
	{
		if (!SystemConfig) return false;
		return (bIsAuthority && SystemConfig->AvailableOnLocalServer) || 
			   (!bIsAuthority && SystemConfig->AvailableOnLocalClient);
	}
	
public:
	void TakeDamage(FFluxPrimeArchetypeDataStore& dataStore, int32 indexDataID, int32 indexData, int16 damageTaken)
	{
		FSystemExecutor& Executor = GetSystemExecutor(CrowdsHealthSystemIndex);
		FFluxPrimeHealthSystem& system = Executor.Data->GetMutable<FFluxPrimeHealthSystem>();
		system.TakeDamage(dataStore, indexDataID, indexData, damageTaken);
	}
	
	void ShootRay(TArray<FFluxPrimeArchetypeDataStore>& dataStores, FVector startRay, FVector endRay, 
		int32& outEntityType, int32& outEntityIndex)
	{
		FSystemExecutor& Executor = GetSystemExecutor(CrowdsRaycastSystemIndex);
		FFluxPrimeRaycastSystem& system = Executor.Data->GetMutable<FFluxPrimeRaycastSystem>();
		system.ShootRay(dataStores, startRay, endRay, outEntityType, outEntityIndex);
	}
	
	void SyncDataSnapshot(TArray<FFluxPrimeArchetypeDataStore>& dataStores, const TArray<FFluxPrimeSnapshotData>& dataSnapshots)
	{
		FSystemExecutor& Executor = GetSystemExecutor(CrowdsSnapshotSystemIndex);
		FFluxPrimeCrowdsSnapshotSystem& system = Executor.Data->GetMutable<FFluxPrimeCrowdsSnapshotSystem>();
		system.SyncData(dataStores, dataSnapshots);
	}
	
	void InitializeSystems(FFluxPrimeCrowdsSystemsModuleContext context)
	{
		NiagaraCallback = NewObject<UFluxPrimeCrowdsNiagaraCallback>();
		
		{
			FFluxPrimeCrowdsNiagaraCallbackContext contextCallback;
			contextCallback.world = context.world;
			NiagaraCallback->InitializedNiagaraCallback(contextCallback);
		}
		
		SystemFragments = SystemsConfiguration->SystemFragments;
		auto& catalog = *context.entityCatalog;
		
		StartInitializedSystem(SystemFragments.Num());
		
		for (int i = 0; i < SystemFragments.Num(); ++i)
		{
			FInstancedStruct& fragment = SystemFragments[i];
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeCrowdsAbilitySystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystem<FFluxPrimeCrowdsAbilitySystem>(fragment);
				
				TArray<float> damageMapping;
				TArray<FGameplayTag> crowdsCatalogAbilityFragmentIdentity;

				for (auto& pair : catalog)
				{
					auto* identity = Cast<UFluxPrimeCrowdsIdentity>(pair.EntityIdentity);
					
					crowdsCatalogAbilityFragmentIdentity.Add(identity->AbilityIdentity);
					damageMapping.Add(identity->Damage);
				}
			
				FFluxPrimeCrowdsAbilitySystemsContext tempContext;
				tempContext.world = context.world;
				tempContext.catalogAbilityFragmentIdentity = &crowdsCatalogAbilityFragmentIdentity;
				tempContext.niagaraCallback = NiagaraCallback;
				tempContext.dataStores = context.dataStores;
				tempContext.damageMapping = damageMapping;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrowdsAbilitySystem& system = Executor.Data->GetMutable<FFluxPrimeCrowdsAbilitySystem>();
				system.Initialized(tempContext);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeHealthSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystem<FFluxPrimeHealthSystem>(fragment);
				CrowdsHealthSystemIndex = executorIndex;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeHealthSystem& system = Executor.Data->GetMutable<FFluxPrimeHealthSystem>();
				system.Initialized(*context.dataStores);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeBoidsSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystem<FFluxPrimeBoidsSystem>(fragment);
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeBoidsSystem& system = Executor.Data->GetMutable<FFluxPrimeBoidsSystem>();
				system.Initialized(*context.dataStores);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeAnimationSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystem<FFluxPrimeAnimationSystem>(fragment);
				CrowdsAnimationSystemIndex = executorIndex;
				
				TArray<FName> catalogIdentity;

				for (auto& pair : *context.entityCatalog)
				{
					FName identity = Cast<UFluxPrimeCrowdsIdentity>(pair.EntityIdentity)->Identity;
					catalogIdentity.Add(identity);
				}
					
				FFluxPrimeAnimationSystemsContext tempContext;
				tempContext.world = context.world;
				tempContext.componentsISMC = context.crowdsComponents;
				tempContext.nameCatalog = &catalogIdentity;
				tempContext.countCatalog = catalog.Num();
				tempContext.animationSoftRef = context.crowdsAnimationSoftRef;
				tempContext.getStateID = context.getStateID;
				tempContext.getNotifyID = context.getNotifyID;
				tempContext.dataStores = context.dataStores;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeAnimationSystem& system = Executor.Data->GetMutable<FFluxPrimeAnimationSystem>();
				system.Initialized(tempContext);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeCrwodsAnimationNotifySystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystem<FFluxPrimeCrwodsAnimationNotifySystem>(fragment);
				
				FFluxPrimeAnimationNotifySystemsContext tempContext;
				tempContext.world = context.world;
				tempContext.getNotifyID = context.getNotifyID;
				tempContext.dataStores = context.dataStores;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrwodsAnimationNotifySystem& system = Executor.Data->GetMutable<FFluxPrimeCrwodsAnimationNotifySystem>();
				system.Initialized(tempContext);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeCrowdsCompactSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystem<FFluxPrimeCrowdsCompactSystem>(fragment);
				
				FFluxPrimeCrowdsCompactSystemsContext tempContext;
				tempContext.dataStores = context.dataStores;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrowdsCompactSystem& system = Executor.Data->GetMutable<FFluxPrimeCrowdsCompactSystem>();
				system.Initialized(tempContext);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeGroundHeightSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystem<FFluxPrimeGroundHeightSystem>(fragment);
				
				FFluxPrimeGroundHeightSystemsContext tempContext;
				tempContext.world = context.world;
				tempContext.dataStores = context.dataStores;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeGroundHeightSystem& system = Executor.Data->GetMutable<FFluxPrimeGroundHeightSystem>();
				system.Initialized(tempContext);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeMovementSystem>())
	        {
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
	            const int32 executorIndex = RegisterSystem<FFluxPrimeMovementSystem>(fragment);
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeMovementSystem& system = Executor.Data->GetMutable<FFluxPrimeMovementSystem>();
				system.Initialized(*context.dataStores);
	        }
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeNavigationSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystem<FFluxPrimeNavigationSystem>(fragment);
	            CrowdsNavigationSystemIndex = executorIndex;
				UE_LOG(LogTemp, Log, TEXT("FRAGMENT:: navigation index %d"), CrowdsNavigationSystemIndex);
				
				FFluxPrimeNavigationSystemsContext tempContext;
				tempContext.world = context.world;
				tempContext.dataStores = context.dataStores;
	            
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeNavigationSystem& system = Executor.Data->GetMutable<FFluxPrimeNavigationSystem>();
				system.Initialized(tempContext);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeRenderSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystem<FFluxPrimeRenderSystem>(fragment);
	            
				TArray<uint16> catalogTotal;

				for (auto& pair : *context.entityCatalog)
				{
					catalogTotal.Add(pair.TotalEntityOnPool);
				}
			
				FFluxPrimeRenderSystemsContext tempContext;
				tempContext.componentsISMC = context.crowdsComponents;
				tempContext.catalogTotal = &catalogTotal;
				tempContext.dataStores = context.dataStores;
	            
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeRenderSystem& system = Executor.Data->GetMutable<FFluxPrimeRenderSystem>();
				system.Initialized(tempContext);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeCrowdsStateMachineSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystem<FFluxPrimeCrowdsStateMachineSystem>(fragment);
				
				FFluxPrimeCrowdsStateMachineSystemContext tempContext;
				tempContext.getStateID = context.getStateID;
				tempContext.entityCatalog = context.entityCatalog;
				tempContext.dataStores = context.dataStores;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrowdsStateMachineSystem& system = Executor.Data->GetMutable<FFluxPrimeCrowdsStateMachineSystem>();
				system.Initialized(tempContext);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeCrowdsTargetSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystem<FFluxPrimeCrowdsTargetSystem>(fragment);
				CrowdsTargetSystemIndex = executorIndex;
				
				TArray<float> aggroDistance;

				for (auto& pair : catalog)
				{
					aggroDistance.Add(Cast<UFluxPrimeCrowdsIdentity>(pair.EntityIdentity)->AggroDistance);
				}
			
				FFluxPrimeCrowdsTargetSystemsContext tempContext;
				tempContext.targetCatalogs = context.crowdsTarget;
				tempContext.aggroDistance = &aggroDistance;
				tempContext.dataStores = context.dataStores;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrowdsTargetSystem& system = Executor.Data->GetMutable<FFluxPrimeCrowdsTargetSystem>();
				system.Initialized(tempContext);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeVisualInterpolationSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystem<FFluxPrimeVisualInterpolationSystem>(fragment);
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeVisualInterpolationSystem& system = Executor.Data->GetMutable<FFluxPrimeVisualInterpolationSystem>();
				system.Initialized(*context.dataStores);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeCrowdsSnapshotSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystem<FFluxPrimeCrowdsSnapshotSystem>(fragment);
				CrowdsSnapshotSystemIndex = executorIndex;
				
				FFluxPrimeSnapshotSystemContext tempContext;
				tempContext.dataStores = context.dataStores;
				tempContext.world = context.world;
				tempContext.hasAuthority = context.hasAuthority;
				tempContext.onManagerActionChange = context.onCrowdsManagerActionChange;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrowdsSnapshotSystem& system = Executor.Data->GetMutable<FFluxPrimeCrowdsSnapshotSystem>();
				system.Initialized(tempContext);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeRaycastSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystem<FFluxPrimeRaycastSystem>(fragment);
				CrowdsRaycastSystemIndex = executorIndex;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeRaycastSystem& system = Executor.Data->GetMutable<FFluxPrimeRaycastSystem>();
				system.Initialized(*context.dataStores);
			}
		}
		
		EndInitializedSystem();
	}
};