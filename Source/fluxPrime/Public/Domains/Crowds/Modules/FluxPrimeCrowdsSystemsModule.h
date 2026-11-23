#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Domains/Crowds/Configurations/SystemsConfiguration.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsIdentity.h"
#include "Framework/Core/FluxPrimeBaseModule.h"
#include "Generics/Systems/FluxPrimeBoidsSystem.h"
#include "Generics/Systems/FluxPrimeGroundHeightSystem.h"
#include "Generics/Systems/FluxPrimeMovementSystem.h"
#include "Generics/Systems/Navigation/FluxPrimeNavigationSystem.h"
#include "Generics/Systems/Animation/FluxPrimeAnimationSystem.h"
#include "Generics/Systems/FluxPrimeRenderSystem.h"
#include "Generics/Systems/FluxPrimeHealthSystem.h"
#include "Domains/Crowds/Systems/FluxPrimeCrowdsCompactSystem.h"
#include "Domains/Crowds/Systems/FluxPrimeCrowdsAbilitySystem.h"
#include "Domains/Crowds/Systems/FluxPrimeCrwodsAnimationNotifySystem.h"
#include "Domains/Crowds/Systems/State/FluxPrimeCrowdsStateMachineSystem.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsNiagaraCallback.h"
#include "Domains/Crowds/Systems/FluxPrimeCrowdsKillEventSystem.h"
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
	/** Catalog entries already resolved to UFluxPrimeCrowdsIdentity, in archetype order. */
	TArray<TObjectPtr<UFluxPrimeCrowdsIdentity>>* resolvedIdentities = nullptr;
	/** One validated animation asset per archetype, in the same order. */
	TArray<TObjectPtr<UFluxPrimeAnimationData>>* resolvedAnimationData = nullptr;
	
	FOnCrowdsManagerActionChange* onCrowdsManagerActionChange = nullptr;
	TArray<FFluxPrimeTargetCatalog>* crowdsTarget = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
	
	bool hasAuthority = false;
	/** Size of the manager's global state table, shared by animation and state-machine registration. */
	int32 globalStateCount = 0;
	
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

	/** Executor indices per system type, in registration (asset) order. Duplicate types are supported. */
	TMap<const UScriptStruct*, TArray<int32>> SystemFragmentsIndex;
		
private:
	template <typename T>
	bool CanRunInCurrentAuthority(const T* SystemConfig, bool bIsAuthority) 
	{
		if (!SystemConfig) return false;
		return (bIsAuthority && SystemConfig->AvailableOnLocalServer) || 
			   (!bIsAuthority && SystemConfig->AvailableOnLocalClient);
	}
	
	template<typename T>
	int32 RegisterSystemFragment(FInstancedStruct& fragment)
	{
		const int32 executorIndex = RegisterSystem<T>(fragment);
		SystemFragmentsIndex.FindOrAdd(T::StaticStruct()).Add(executorIndex);
		return executorIndex;
	}
	
public:
	/**
	 * Returns the first registered instance of T in asset order. Duplicate fragments of the same
	 * system type are allowed: every instance is registered and executes, and this accessor is
	 * deterministic rather than arbitrary. Use GetSystems<T>() when all instances matter.
	 */
	template<typename T>
	T* GetSystem()
	{
		if (const TArray<int32>* executorIndices = SystemFragmentsIndex.Find(T::StaticStruct()))
		{
			if (executorIndices->IsEmpty()) return nullptr;
			FSystemExecutor& executor = GetSystemExecutor((*executorIndices)[0]);
			return executor.Data->GetMutablePtr<T>();
		}
		
		return nullptr;
	}

	/** Returns every registered instance of T in asset order. */
	template<typename T>
	void GetSystems(TArray<T*>& outSystems)
	{
		outSystems.Reset();

		const TArray<int32>* executorIndices = SystemFragmentsIndex.Find(T::StaticStruct());
		if (!executorIndices) return;

		for (const int32 executorIndex : *executorIndices)
		{
			FSystemExecutor& executor = GetSystemExecutor(executorIndex);
			if (T* system = executor.Data->GetMutablePtr<T>()) outSystems.Add(system);
		}
	}

	/** Registration order of the first instance of T, or INDEX_NONE when it is not active. */
	template<typename T>
	int32 GetSystemOrder() const
	{
		const TArray<int32>* executorIndices = SystemFragmentsIndex.Find(T::StaticStruct());
		if (!executorIndices || executorIndices->IsEmpty()) return INDEX_NONE;
		return (*executorIndices)[0];
	}

	/**
	 * Visual interpolation currently writes the simulation position buffer, so its order is part of
	 * gameplay behavior. See Docs/DECISIONS/REF-032-visual-interpolation.md.
	 */
	void ValidateSystemOrder() const
	{
		const int32 InterpolationOrder = GetSystemOrder<FFluxPrimeVisualInterpolationSystem>();
		if (InterpolationOrder == INDEX_NONE) return;

		auto RequireBefore = [this, InterpolationOrder](int32 PrerequisiteOrder, const TCHAR* Name)
		{
			if (PrerequisiteOrder == INDEX_NONE || PrerequisiteOrder < InterpolationOrder) return;

			UE_LOG(LogFluxPrime, Error,
				TEXT("System order: %s (index %d) must run before FFluxPrimeVisualInterpolationSystem (index %d). ")
				TEXT("Interpolation writes the simulation position buffer, so this order changes gameplay. ")
				TEXT("See Docs/DECISIONS/REF-032-visual-interpolation.md."),
				Name, PrerequisiteOrder, InterpolationOrder);
		};

		RequireBefore(GetSystemOrder<FFluxPrimeMovementSystem>(), TEXT("FFluxPrimeMovementSystem"));
		RequireBefore(GetSystemOrder<FFluxPrimeGroundHeightSystem>(), TEXT("FFluxPrimeGroundHeightSystem"));
		RequireBefore(GetSystemOrder<FFluxPrimeRaycastSystem>(), TEXT("FFluxPrimeRaycastSystem"));
		RequireBefore(GetSystemOrder<FFluxPrimeCrowdsTargetSystem>(), TEXT("FFluxPrimeCrowdsTargetSystem"));
		RequireBefore(GetSystemOrder<FFluxPrimeCrowdsSnapshotSystem>(), TEXT("FFluxPrimeCrowdsSnapshotSystem"));

		const int32 RenderOrder = GetSystemOrder<FFluxPrimeRenderSystem>();
		if (RenderOrder != INDEX_NONE && RenderOrder < InterpolationOrder)
		{
			UE_LOG(LogFluxPrime, Error,
				TEXT("System order: FFluxPrimeRenderSystem (index %d) must run after FFluxPrimeVisualInterpolationSystem (index %d). ")
				TEXT("Otherwise rendering is one frame behind the interpolated position."),
				RenderOrder, InterpolationOrder);
		}
	}
	
	void InitializeSystems(FFluxPrimeCrowdsSystemsModuleContext context)
	{
		IsAllDataValid = false;
		StartInitializedSystem(0);
		SystemFragmentsIndex.Reset();
		if (!SystemsConfiguration)
		{
			UE_LOG(LogFluxPrime, Error, TEXT("Crowds systems module cannot initialize: SystemsConfiguration is not set."));
			return;
		}

		if (!context.resolvedIdentities || !context.entityCatalog || !context.resolvedAnimationData ||
			!context.dataStores ||
			context.resolvedIdentities->Num() != context.entityCatalog->Num() ||
			context.resolvedAnimationData->Num() != context.entityCatalog->Num() ||
			context.dataStores->Num() != context.entityCatalog->Num())
		{
			UE_LOG(LogFluxPrime, Error,
				TEXT("Crowds systems module cannot initialize: resolved identities (%d), animations (%d) or stores (%d) ")
				TEXT("do not match the catalog (%d)."),
				context.resolvedIdentities ? context.resolvedIdentities->Num() : 0,
				context.resolvedAnimationData ? context.resolvedAnimationData->Num() : 0,
				context.dataStores ? context.dataStores->Num() : 0,
				context.entityCatalog ? context.entityCatalog->Num() : 0);
			return;
		}
		
		SystemFragments = SystemsConfiguration->SystemFragments;
		
		SystemFragmentsIndex.Reset();
		StartInitializedSystem(SystemFragments.Num());
		
		for (int i = 0; i < SystemFragments.Num(); ++i)
		{
			FInstancedStruct& fragment = SystemFragments[i];
			// All three consumers share the same asset-owned orientation for this module's lifetime.
			if (auto* Movement = fragment.GetMutablePtr<FFluxPrimeMovementSystem>())
				Movement->MeshOrientation = &SystemsConfiguration->MeshOrientation;
			if (auto* Interpolation = fragment.GetMutablePtr<FFluxPrimeVisualInterpolationSystem>())
				Interpolation->MeshOrientation = &SystemsConfiguration->MeshOrientation;
			if (auto* Ability = fragment.GetMutablePtr<FFluxPrimeCrowdsAbilitySystem>())
				Ability->MeshOrientation = &SystemsConfiguration->MeshOrientation;
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeCrowdsAbilitySystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeCrowdsAbilitySystem>(fragment);
				
				TArray<float> damageMapping;
				TArray<FGameplayTag> crowdsCatalogAbilityFragmentIdentity;

				for (const TObjectPtr<UFluxPrimeCrowdsIdentity>& identity : *context.resolvedIdentities)
				{
					crowdsCatalogAbilityFragmentIdentity.Add(identity->AbilityIdentity);
					damageMapping.Add(identity->Damage);
				}
			
				FFluxPrimeCrowdsAbilitySystemsContext tempContext;
				tempContext.world = context.world;
				tempContext.catalogAbilityFragmentIdentity = &crowdsCatalogAbilityFragmentIdentity;
				tempContext.dataStores = context.dataStores;
				tempContext.damageMapping = damageMapping;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrowdsAbilitySystem& system = Executor.Data->GetMutable<FFluxPrimeCrowdsAbilitySystem>();
				system.Initialized(tempContext);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeHealthSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeHealthSystem>(fragment);
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeHealthSystem& system = Executor.Data->GetMutable<FFluxPrimeHealthSystem>();
				system.Initialized(*context.dataStores);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeBoidsSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeBoidsSystem>(fragment);
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeBoidsSystem& system = Executor.Data->GetMutable<FFluxPrimeBoidsSystem>();
				system.Initialized(*context.dataStores);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeAnimationSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeAnimationSystem>(fragment);
				
				FFluxPrimeAnimationSystemsContext tempContext;
				tempContext.world = context.world;
				tempContext.componentsISMC = context.crowdsComponents;
				tempContext.resolvedAnimationData = context.resolvedAnimationData;
				tempContext.getStateID = context.getStateID;
				tempContext.getNotifyID = context.getNotifyID;
				tempContext.globalStateCount = context.globalStateCount;
				tempContext.dataStores = context.dataStores;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeAnimationSystem& system = Executor.Data->GetMutable<FFluxPrimeAnimationSystem>();
				system.Initialized(tempContext);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeCrwodsAnimationNotifySystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeCrwodsAnimationNotifySystem>(fragment);
				
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
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeCrowdsCompactSystem>(fragment);
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrowdsCompactSystem& system = Executor.Data->GetMutable<FFluxPrimeCrowdsCompactSystem>();
				system.Initialized(*context.dataStores);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeGroundHeightSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeGroundHeightSystem>(fragment);
				
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
				
	            const int32 executorIndex = RegisterSystemFragment<FFluxPrimeMovementSystem>(fragment);
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeMovementSystem& system = Executor.Data->GetMutable<FFluxPrimeMovementSystem>();
				system.Initialized(*context.dataStores);
	        }
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeNavigationSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeNavigationSystem>(fragment);
				
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
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeRenderSystem>(fragment);
	            
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
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeCrowdsStateMachineSystem>(fragment);
				
				FFluxPrimeCrowdsStateMachineSystemContext tempContext;
				tempContext.getStateID = context.getStateID;
				tempContext.globalStateCount = context.globalStateCount;
				tempContext.entityCatalog = context.entityCatalog;
				tempContext.dataStores = context.dataStores;
				tempContext.meshOrientation = &SystemsConfiguration->MeshOrientation;
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrowdsStateMachineSystem& system = Executor.Data->GetMutable<FFluxPrimeCrowdsStateMachineSystem>();
				system.Initialized(tempContext);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeCrowdsTargetSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeCrowdsTargetSystem>(fragment);
				
				TArray<float> aggroDistance;

				for (const TObjectPtr<UFluxPrimeCrowdsIdentity>& identity : *context.resolvedIdentities)
				{
					aggroDistance.Add(identity->AggroDistance);
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
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeVisualInterpolationSystem>(fragment);
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeVisualInterpolationSystem& system = Executor.Data->GetMutable<FFluxPrimeVisualInterpolationSystem>();
				system.Initialized(*context.dataStores);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeCrowdsSnapshotSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeCrowdsSnapshotSystem>(fragment);
				
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
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeRaycastSystem>(fragment);
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeRaycastSystem& system = Executor.Data->GetMutable<FFluxPrimeRaycastSystem>();
				system.Initialized(*context.dataStores);
			}
			
			if (auto* pairSystem = fragment.GetPtr<FFluxPrimeCrowdsKillEventSystem>())
			{
				if (!CanRunInCurrentAuthority(pairSystem, context.hasAuthority)) continue;
				
				const int32 executorIndex = RegisterSystemFragment<FFluxPrimeCrowdsKillEventSystem>(fragment);
				
				FSystemExecutor& Executor = GetSystemExecutor(executorIndex);
				FFluxPrimeCrowdsKillEventSystem& system = Executor.Data->GetMutable<FFluxPrimeCrowdsKillEventSystem>();
				system.Initialized(*context.dataStores);
			}
		}
		
		// Duplicate animation fragments share the same ISM buffer; relocate it once per move.
		FFluxPrimeAnimationSystem* Animation = GetSystem<FFluxPrimeAnimationSystem>();
		TArray<FFluxPrimeCrowdsCompactSystem*> CompactSystems;
		GetSystems(CompactSystems);
		for (FFluxPrimeCrowdsCompactSystem* Compact : CompactSystems)
		{
			Compact->RelocatePresentation = Animation ? Animation->CreatePresentationRelocator()
				: TFunction<void(int32, int32, int32)>();
		}

		ValidateSystemOrder();

		IsAllDataValid = true;
		EndInitializedSystem();
	}
};