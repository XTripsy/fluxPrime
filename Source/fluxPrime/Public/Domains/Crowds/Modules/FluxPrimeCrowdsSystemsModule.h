#pragma once

#include "CoreMinimal.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsData.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Framework/Core/FluxPrimeBaseModule.h"
#include "Generics/Systems/FluxPrimeBoidsSystem.h"
#include "Generics/Systems/FluxPrimeGroundHeightSystem.h"
#include "Generics/Systems/FluxPrimeMovementSystem.h"
#include "Generics/Systems/FluxPrimeNavigationSystem.h"
#include "Generics/Systems/Animation/FluxPrimeAnimationSystem.h"
#include "Domains/Crowds/Systems/FluxPrimeCrowdsCompactSystem.h"
#include "Generics/Systems/FluxPrimeRenderSystem.h"
#include "Domains/Crowds/Systems/Ability/FluxPrimeCrowdsAbilitySystem.h"
#include "Generics/Systems/FluxPrimeHealthSystem.h"
#include "Domains/Crowds/Systems/FluxPrimeCrowdsTargetSystem.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Domains/Crowds/Systems/Notify/FluxPrimeCrwodsAnimationNotifySystem.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsNiagaraCallback.h"
#include "Domains/Crowds/Systems/State/FluxPrimeCrowdsStateMachineSystem.h"
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
	
	FGetStateID getStateID = nullptr;
	FGetNotifyID getNotifyID = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsSystemsModule : public FFluxPrimeBaseModule
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere, meta = (BaseStruct="FluxPrimeBaseSystem", ExcludeBaseStruct, AllowPrivateAccess = true))
	TArray<FInstancedStruct> SystemFragments;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsNiagaraCallback> NiagaraCallback = nullptr;
	
	int32 CrowdsHealthSystemIndex = INDEX_NONE, CrowdsTargetSystemIndex = INDEX_NONE, CrowdsNavigationSystemIndex = INDEX_NONE;
	
public:
	void TakeDamage(FFluxPrimeArchetypeDataStore& dataStore, int32 indexDataID, int32 indexData, int16 damageTaken)
	{
		FSystemExecutor& Executor = GetSystemExecutor(CrowdsHealthSystemIndex);
		FFluxPrimeHealthSystem& system = Executor.Data->GetMutable<FFluxPrimeHealthSystem>();
		system.TakeDamage(dataStore, indexDataID, indexData, damageTaken);
	}
	
	void SyncNewTarget(FFluxPrimeArchetypeDataStore& dataStore, int32 indexDataID, int32 indexData, 
		int16 newTargetID, FVector newTargetLocation, bool newRequestNeedReplan)
	{
		FSystemExecutor& Executor = GetSystemExecutor(CrowdsTargetSystemIndex);
		FFluxPrimeCrowdsTargetSystem& targetSystem = Executor.Data->GetMutable<FFluxPrimeCrowdsTargetSystem>();
		targetSystem.SyncNewTarget(dataStore, indexDataID, indexData, newTargetID, newTargetLocation);
		
		Executor = GetSystemExecutor(CrowdsNavigationSystemIndex);
		FFluxPrimeNavigationSystem& navigationSystem = Executor.Data->GetMutable<FFluxPrimeNavigationSystem>();
		navigationSystem.SyncNewTarget(dataStore, indexDataID, indexData, newRequestNeedReplan);
	}
	
	void InitializeSystems(FFluxPrimeCrowdsSystemsModuleContext context)
	{
		NiagaraCallback = NewObject<UFluxPrimeCrowdsNiagaraCallback>();
		
		{
			FFluxPrimeCrowdsNiagaraCallbackContext contextCallback;
			contextCallback.world = context.world;
			NiagaraCallback->InitializedNiagaraCallback(contextCallback);
		}
		
		auto& catalog = *context.entityCatalog;
		
		StartInitializedSystem(SystemFragments.Num());
		
		for (int i = 0; i < SystemFragments.Num(); ++i)
		{
			FInstancedStruct& fragment = SystemFragments[i];
			
			if (fragment.GetPtr<FFluxPrimeCrowdsAbilitySystem>())
			{
				const int32 executorIndex = RegisterSystem<FFluxPrimeCrowdsAbilitySystem>(fragment);
				
				TArray<TObjectPtr<UScriptStruct>> crowdsCatalogFragment;
				TArray<FGuid> crowdsCatalogFragmentID;

				for (auto& pair : catalog)
				{
					FAbilityFragment temp = Cast<UFluxPrimeCrowdsIdentity>(pair.EntityIdentity)->AbilityFragment;
				
					crowdsCatalogFragment.Add(temp.AbilityFragment);
					crowdsCatalogFragmentID.Add(temp.ID);
				}
			
				FFluxPrimeCrowdsAbilitySystemsContext tempContext;
				tempContext.world = context.world;
				tempContext.catalogFragment = &crowdsCatalogFragment;
				tempContext.catalogFragmentID = &crowdsCatalogFragmentID;
				tempContext.niagaraCallback = NiagaraCallback;
				tempContext.dataStores = context.dataStores;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrowdsAbilitySystem& system = Executor.Data->GetMutable<FFluxPrimeCrowdsAbilitySystem>();
				system.Initialized(tempContext);
			}
			
			if (fragment.GetPtr<FFluxPrimeHealthSystem>())
			{
				const int32 executorIndex = RegisterSystem<FFluxPrimeHealthSystem>(fragment);
				CrowdsHealthSystemIndex = executorIndex;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeHealthSystem& system = Executor.Data->GetMutable<FFluxPrimeHealthSystem>();
				system.Initialized(*context.dataStores);
			}
			
			if (fragment.GetPtr<FFluxPrimeBoidsSystem>())
			{
				const int32 executorIndex = RegisterSystem<FFluxPrimeBoidsSystem>(fragment);
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeBoidsSystem& system = Executor.Data->GetMutable<FFluxPrimeBoidsSystem>();
				system.Initialized(*context.dataStores);
			}
			
			if (fragment.GetPtr<FFluxPrimeAnimationSystem>())
			{
				const int32 executorIndex = RegisterSystem<FFluxPrimeAnimationSystem>(fragment);
				
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
			
			if (fragment.GetPtr<FFluxPrimeCrwodsAnimationNotifySystem>())
			{
				const int32 executorIndex = RegisterSystem<FFluxPrimeCrwodsAnimationNotifySystem>(fragment);
				
				FFluxPrimeAnimationNotifySystemsContext tempContext;
				tempContext.getNotifyID = context.getNotifyID;
				tempContext.dataStores = context.dataStores;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrwodsAnimationNotifySystem& system = Executor.Data->GetMutable<FFluxPrimeCrwodsAnimationNotifySystem>();
				system.Initialized(tempContext);
			}
			
			if (fragment.GetPtr<FFluxPrimeCrowdsCompactSystem>())
			{
				const int32 executorIndex = RegisterSystem<FFluxPrimeCrowdsCompactSystem>(fragment);
				
				FFluxPrimeCrowdsCompactSystemsContext tempContext;
				tempContext.dataStores = context.dataStores;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrowdsCompactSystem& system = Executor.Data->GetMutable<FFluxPrimeCrowdsCompactSystem>();
				system.Initialized(tempContext);
			}
			
			if (fragment.GetPtr<FFluxPrimeGroundHeightSystem>())
			{
				const int32 executorIndex = RegisterSystem<FFluxPrimeGroundHeightSystem>(fragment);
				
				FFluxPrimeGroundHeightSystemsContext tempContext;
				tempContext.world = context.world;
				tempContext.dataStores = context.dataStores;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeGroundHeightSystem& system = Executor.Data->GetMutable<FFluxPrimeGroundHeightSystem>();
				system.Initialized(tempContext);
			}
			
			if (fragment.GetPtr<FFluxPrimeMovementSystem>())
	        {
	            const int32 executorIndex = RegisterSystem<FFluxPrimeMovementSystem>(fragment);
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeMovementSystem& system = Executor.Data->GetMutable<FFluxPrimeMovementSystem>();
				system.Initialized(*context.dataStores);
	        }
			
			if (fragment.GetPtr<FFluxPrimeNavigationSystem>())
			{
				const int32 executorIndex = RegisterSystem<FFluxPrimeNavigationSystem>(fragment);
	            CrowdsNavigationSystemIndex = executorIndex;
				
				FFluxPrimeNavigationSystemsContext tempContext;
				tempContext.world = context.world;
				tempContext.dataStores = context.dataStores;
	            
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeNavigationSystem& system = Executor.Data->GetMutable<FFluxPrimeNavigationSystem>();
				system.Initialized(tempContext);
			}
			
			if (fragment.GetPtr<FFluxPrimeRenderSystem>())
			{
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
			
			if (fragment.GetPtr<FFluxPrimeCrowdsStateMachineSystem>())
			{
				const int32 executorIndex = RegisterSystem<FFluxPrimeCrowdsStateMachineSystem>(fragment);
				
				FFluxPrimeCrowdsStateMachineSystemContext tempContext;
				tempContext.getStateID = context.getStateID;
				tempContext.entityCatalog = context.entityCatalog;
				tempContext.dataStores = context.dataStores;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrowdsStateMachineSystem& system = Executor.Data->GetMutable<FFluxPrimeCrowdsStateMachineSystem>();
				system.Initialized(tempContext);
			}
			
			if (fragment.GetPtr<FFluxPrimeCrowdsTargetSystem>())
			{
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
				tempContext.onManagerActionChange = context.onCrowdsManagerActionChange;
				tempContext.dataStores = context.dataStores;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrowdsTargetSystem& system = Executor.Data->GetMutable<FFluxPrimeCrowdsTargetSystem>();
				system.Initialized(tempContext);
			}
		}
		
		EndInitializedSystem();
	}
};