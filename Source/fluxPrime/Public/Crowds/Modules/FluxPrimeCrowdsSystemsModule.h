#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Crowds/ManagerConfiguration/ManagerConfiguration.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationAnimationSystem.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationBoidsSystem.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationAbilitySystem.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationGroundHeightSystem.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationHealthSystem.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationMovementSystem.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationNavigationSystem.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationProxyTargetSystem.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationSpatialGridSystem.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationStateMachineSystem.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationTargetSystem.h"
#include "Systems/FluxPrimeBoidsSystem.h"
#include "Systems/FluxPrimeGroundHeightSystem.h"
#include "Systems/FluxPrimeMovementSystem.h"
#include "Systems/FluxPrimeNavigationSystem.h"
#include "Systems/FluxPrimeSpatialGridSystem.h"
#include "Systems/FluxPrimeAnimationSystem.h"
#include "Systems/FluxPrimeCompactSystem.h"
#include "Systems/FluxPrimeRenderSystem.h"
#include "Systems/FluxPrimeAbilitySystem.h"
#include "Systems/FluxPrimeHealthSystem.h"
#include "Systems/FluxPrimeStateMachineSystem.h"
#include "Systems/FluxPrimeTargetSystem.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Systems/FluxPrimeAnimationNotifySystem.h"
#include "Cores/FluxPrimeNiagaraCallback.h"
#include "FluxPrimeCrowdsSystemsModule.generated.h"

USTRUCT()
struct FFluxPrimeCrowdsSystemsComponentContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UManagerConfiguration> managerConfiguration;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* crowdsComponents = nullptr;
	TArray<FFluxPrimeCrowdsCatalog>* crowdsCatalog = nullptr;
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>>* crowdsAnimationSoftRef = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	FFluxPrimeCrowds* crowdsDatas = nullptr;
	uint16* crowdsActive = nullptr;
	uint16* crowdsTotal = nullptr;
	TArray<TArray<int16>>* crowdsPool = nullptr;
	TArray<int16>* crowdsHeadPool = nullptr;
	TMap<FFluxPrimeCrowdsLookup, int32>* crowdsLookup = nullptr;
	FOnCrowdsManagerActionChange* onCrowdsManagerActionChange = nullptr;
	//TArray<TWeakObjectPtr<AActor>>* crowdsTarget = nullptr;
	TArray<FFluxPrimeTargetCatalog>* crowdsTarget = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsSystemsModule
{
	GENERATED_BODY()
	
private:
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* CrowdsComponents = nullptr;
	TArray<FFluxPrimeCrowdsCatalog>* CrowdsCatalog = nullptr;
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>>* CrowdsAnimationSoftRef = nullptr;
	
	UPROPERTY()
	TObjectPtr<UManagerConfiguration> ManagerConfiguration;
	
	FFluxPrimeRenderSystem CrowdsRenderSystems;
	FFluxPrimeSpatialGridSystem SpatialGridSystems;
	FFluxPrimeBoidsSystem BoidsSystems;
	FFluxPrimeMovementSystem MovementSystems;
	FFluxPrimeNavigationSystem NavigationSystems;
	FFluxPrimeGroundHeightSystem GroundHeightSystems;
	FFluxPrimeAnimationSystem AnimationSystems;
	FFluxPrimeAnimationNotifySystem AnimationNotifySystems;
	FFluxPrimeStateMachineSystem StateMachineSystems;
	FFluxPrimeHealthSystem HealthSystems;
	FFluxPrimeTargetSystem TargetSystems;
	FFluxPrimeAbilitySystem AbilitySystems;
	FFluxPrimeCompactSystem CompactSystems;
	
	FFluxPrimeCrowds* CrowdsDatas = nullptr;
	uint16* CrowdsActive = nullptr;
	uint16* CrowdsTotal = nullptr;
	TArray<TArray<int16>>* CrowdsPool = nullptr;
	TArray<int16>* CrowdsHeadPool = nullptr;
	TMap<FFluxPrimeCrowdsLookup, int32>* CrowdsLookup = nullptr;
	FOnCrowdsManagerActionChange* OnCrowdsManagerActionChange = nullptr;
	//TArray<TWeakObjectPtr<AActor>>* CrowdsTarget = nullptr;
	TArray<FFluxPrimeTargetCatalog>* CrowdsTarget = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeNiagaraCallback> NiagaraCallback = nullptr;
	
public:
	void Damage(uint32 index)
	{
		HealthSystems.OnHitHealthSystems(index);
	}
	
	void Initialize(FFluxPrimeCrowdsSystemsComponentContext context)
	{
		check(context.world);
		check(context.crowdsDatas);
		check(context.crowdsPool);
		check(context.crowdsHeadPool);
		check(context.crowdsActive);
		check(context.crowdsLookup);
		check(context.crowdsCatalog);
		check(context.crowdsAnimationSoftRef);
		check(context.onCrowdsManagerActionChange);
		check(context.crowdsTarget);
		
		CrowdsComponents = context.crowdsComponents;
		ManagerConfiguration = context.managerConfiguration;
		CrowdsCatalog = context.crowdsCatalog;
		CrowdsAnimationSoftRef = context.crowdsAnimationSoftRef;
		
		CrowdsDatas = context.crowdsDatas;
		CrowdsActive = context.crowdsActive;
		CrowdsTotal = context.crowdsTotal;
		CrowdsPool = context.crowdsPool;
		CrowdsHeadPool = context.crowdsHeadPool;
		CrowdsLookup = context.crowdsLookup;
		OnCrowdsManagerActionChange = context.onCrowdsManagerActionChange;
		CrowdsTarget = context.crowdsTarget;
		
		World = context.world;
		NiagaraCallback = NewObject<UFluxPrimeNiagaraCallback>();
		
		{
			FFluxPrimeNiagaraCallbackContext contextCallback;
			contextCallback.world = World;
			NiagaraCallback->InitializedNiagaraCallback(contextCallback);
		}
	}

	void InitializeSystems()
	{
		const FFluxPrimeConfigurationGroundHeightSystem* configurationGroundHeight = nullptr;
		const FFluxPrimeConfigurationSpatialGridSystem* configurationSpatialGrid = nullptr;
		const FFluxPrimeConfigurationNavigationSystem* configurationNavigation = nullptr;
		const FFluxPrimeConfigurationBoidsSystem* configurationBoids = nullptr;
		const FFluxPrimeConfigurationMovementSystem* configurationMovement = nullptr;
		const FFluxPrimeConfigurationAnimationSystem* configurationAnimation = nullptr;
		const FFluxPrimeConfigurationProxyTargetSystem* configurationProxyTarget = nullptr;
		const FFluxPrimeConfigurationStateMachineSystem* configurationStateMachine = nullptr;
		const FFluxPrimeConfigurationTargetSystem* configurationTarget = nullptr;
		const FFluxPrimeConfigurationHealthSystem* configurationHealth = nullptr;
		const FFluxPrimeConfigurationAbilitySystem* configurationAbility = nullptr;
		
		for (auto& pair : ManagerConfiguration->ConfigurationFragments)
		{
			if (!pair.IsValid()) continue;
			
			if (const FFluxPrimeConfigurationGroundHeightSystem* configuration = pair.GetPtr<FFluxPrimeConfigurationGroundHeightSystem>())
			{
				GroundHeightSystems.IsActive = true;
				configurationGroundHeight = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationSpatialGridSystem* configuration = pair.GetPtr<FFluxPrimeConfigurationSpatialGridSystem>())
			{
				configurationSpatialGrid = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationNavigationSystem* configuration = pair.GetPtr<FFluxPrimeConfigurationNavigationSystem>())
			{
				NavigationSystems.IsActive = true;
				configurationNavigation = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationBoidsSystem* configuration = pair.GetPtr<FFluxPrimeConfigurationBoidsSystem>())
			{
				BoidsSystems.IsActive = true;
				configurationBoids = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationMovementSystem* configuration = pair.GetPtr<FFluxPrimeConfigurationMovementSystem>())
			{
				MovementSystems.IsActive = true;
				configurationMovement = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationAnimationSystem* configuration = pair.GetPtr<FFluxPrimeConfigurationAnimationSystem>())
			{
				AnimationSystems.IsActive = true;
				configurationAnimation = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationAbilitySystem* configuration = pair.GetPtr<FFluxPrimeConfigurationAbilitySystem>())
			{
				AbilitySystems.IsActive = true;
				configurationAbility = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationStateMachineSystem* configuration = pair.GetPtr<FFluxPrimeConfigurationStateMachineSystem>())
			{
				StateMachineSystems.IsActive = true;
				configurationStateMachine = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationTargetSystem* configuration = pair.GetPtr<FFluxPrimeConfigurationTargetSystem>())
			{
				TargetSystems.IsActive = true;
				configurationTarget = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationHealthSystem* configuration = pair.GetPtr<FFluxPrimeConfigurationHealthSystem>())
			{
				HealthSystems.IsActive = true;
				configurationHealth = configuration;
				continue;
			}
		}
		
	#if !WITH_EDITOR
		IsShowDebug = false;
		if (configurationSpatialGrid) configurationSpatialGrid->IsDebug = false;
		if (configurationNavigation) configurationNavigation->IsDebug = false;
		if (configurationAnimation) configurationAnimation->IsDebug = false;
		if (configurationMovement) configurationMovement->IsDebug = false;
		if (configurationStateMachine) configurationStateMachine->IsDebug = false;
	#endif
		
		auto& member = *CrowdsDatas;
		
		if (GroundHeightSystems.IsActive)
		{
			FFluxPrimeGroundHeightSystemsContext context;
			context.locationCrowds = &member.CrowdsLocation;
			context.rotationCrowds = &member.CrowdsRotation;
			context.maxSpeedCrowds = &member.CrowdsMaxSpeed;
			context.memberActive = CrowdsActive;
			context.world = World;
			context.origin = configurationGroundHeight->Origin;
			context.cellHeight = configurationGroundHeight->CellHeight;
			context.cellWidth = configurationGroundHeight->CellWidth;
			context.cellSize = configurationGroundHeight->CellSize;
			
			GroundHeightSystems.InitializedGroundHeightSystems(context);
			GroundHeightSystems.BakeGroundHeightSystems();
		}

		if (NavigationSystems.IsActive)
		{
			FFluxPrimeSpatialGridSystemsContext contextSpatialGrid;
			contextSpatialGrid.world = World;
			contextSpatialGrid.locationCrowds = &member.CrowdsLocation;
			contextSpatialGrid.cellIDCrowds = &member.CrowdsCellID;
			contextSpatialGrid.memberActive = CrowdsActive;
			contextSpatialGrid.debugColor = configurationNavigation->ConfigurationSpatialGrid.Color;
			contextSpatialGrid.isDebug = configurationNavigation->ConfigurationSpatialGrid.IsDebug;
			contextSpatialGrid.cellHeight = configurationNavigation->ConfigurationSpatialGrid.CellHeight;
			contextSpatialGrid.cellWidth = configurationNavigation->ConfigurationSpatialGrid.CellWidth;
			contextSpatialGrid.cellSize = configurationNavigation->ConfigurationSpatialGrid.CellSize;
			contextSpatialGrid.origin = configurationNavigation->ConfigurationSpatialGrid.Origin;
			
			FFluxPrimeNavigationSystemsContext context;
			context.isDebug = configurationNavigation->IsDebug;
			context.queuePathCountPerFrame = configurationNavigation->QueuePathCountPerFrame;
			context.queueCorridorCountPerFrame = configurationNavigation->QueueCorridorCountPerFrame;
			context.optimizeTimeSameCellID = configurationNavigation->OptimizeTimeSameCellID;
			context.optimizeTimeDifferenceCellID = configurationNavigation->OptimizeTimeDifferenceCellID;
			context.moveTargetTimeSameCellID = configurationNavigation->MoveTargetTimeSameCellID;
			context.moveTargetTimeDifferenceCellID = configurationNavigation->MoveTargetTimeDifferenceCellID;
			context.crowdsCurrentLocation = &member.CrowdsLocation;
			context.crowdsPreviousLocation = &member.CrowdsPreviousLocation;
			context.crowdsSize = &member.CrowdsSize;
			context.crowdsCellID = &member.CrowdsCellID;
			context.crowdsCorridors = &member.CrowdsCorridors;
			context.crowdsTarget = &member.CrowdsTarget;
			context.crowdsCurrentTarget = &member.CrowdsCurrentTarget;
			context.crowdsLastReplanTarget = &member.CrowdsLastReplanTarget;
			context.crowdsLastMoveTarget = &member.CrowdsLastMoveTarget;
			context.crowdsLastOptimizeTime = &member.CrowdsLastOptimizeTime;
			context.crowdsLastMoveTargetTime = &member.CrowdsLastMoveTargetTime;
			context.crowdsCountCorridor = &member.CrowdsCountCorridor;
			context.crowdsNeedReplan = &member.CrowdsRequestNeedReplan;
			context.crowdsWaypoints = &member.CrowdsWaypoints;
			context.crowdsCountWaypoints = &member.CrowdsCountWaypoints;
			context.contextSpatialGrid = contextSpatialGrid;
			
			NavigationSystems.InitializedNavigationSystems(context);
		}
		
		if (BoidsSystems.IsActive)
		{
			FFluxPrimeSpatialGridSystemsContext contextSpatialGrid;
			contextSpatialGrid.world = World;
			contextSpatialGrid.locationCrowds = &member.CrowdsLocation;
			contextSpatialGrid.cellIDCrowds = &member.CrowdsCellID;
			contextSpatialGrid.memberActive = CrowdsActive;
			contextSpatialGrid.debugColor = configurationBoids->ConfigurationSpatialGrid.Color;
			contextSpatialGrid.isDebug = configurationBoids->ConfigurationSpatialGrid.IsDebug;
			contextSpatialGrid.cellHeight = configurationBoids->ConfigurationSpatialGrid.CellHeight;
			contextSpatialGrid.cellWidth = configurationBoids->ConfigurationSpatialGrid.CellWidth;
			contextSpatialGrid.cellSize = configurationBoids->ConfigurationSpatialGrid.CellSize;
			contextSpatialGrid.origin = configurationBoids->ConfigurationSpatialGrid.Origin;
			
			FFluxPrimeBoidsSystemsContext context;
			context.separationWeight = configurationBoids->SeparationWeight;
			context.locationCrowds = &member.CrowdsLocation;
			context.accelerationCrowds = &member.CrowdsAcceleration;
			context.sizeCrowds = &member.CrowdsSize;
			context.contextSpatialGrid = contextSpatialGrid;
			
			BoidsSystems.InitializeBoidsSystems(context);
		}

		if (AnimationSystems.IsActive)
		{
			{
				TArray<FName> catalogIdentity;

				for (auto& pair : *CrowdsCatalog)
				{
					catalogIdentity.Add(pair.CrowdsIdentity->Identity);
				}
				
				FFluxPrimeSpatialGridSystemsContext contextSpatialGrid;
				contextSpatialGrid.world = World;
				contextSpatialGrid.locationCrowds = &member.CrowdsLocation;
				contextSpatialGrid.cellIDCrowds = &member.CrowdsCellID;
				contextSpatialGrid.memberActive = CrowdsActive;
				contextSpatialGrid.debugColor = configurationAnimation->ConfigurationSpatialGrid.Color;
				contextSpatialGrid.isDebug = configurationAnimation->ConfigurationSpatialGrid.IsDebug;
				contextSpatialGrid.cellHeight = configurationAnimation->ConfigurationSpatialGrid.CellHeight;
				contextSpatialGrid.cellWidth = configurationAnimation->ConfigurationSpatialGrid.CellWidth;
				contextSpatialGrid.cellSize = configurationAnimation->ConfigurationSpatialGrid.CellSize;
				contextSpatialGrid.origin = configurationAnimation->ConfigurationSpatialGrid.Origin;
				
				FFluxPrimeAnimationSystemsContext context;
				context.isDebug = configurationAnimation->IsDebug;
				context.contextSpatialGrid = contextSpatialGrid;
				context.crowdsComponents = CrowdsComponents;
				context.crowdsCatalogName = &catalogIdentity;
				context.crowdsCountCatalog = CrowdsCatalog->Num();
				context.crowdsAnimationSoftRef = CrowdsAnimationSoftRef;
				context.world = World;
				context.crowdsCellID = &member.CrowdsCellID;
				context.crowdsCurrentTarget = &member.CrowdsCurrentTarget;
				context.instanceIndexCrowds = &member.CrowdsID;
				context.typeCrowds = &member.CrowdsType;
				context.stateCrowds = &member.CrowdsState;
				context.animationStateCrowds = &member.CrowdsAnimationState;
				context.startTimeAnimationCrowds = &member.CrowdsStartTimeAnimation;
				context.previousAnimationFrameCrowds = &member.CrowdsPreviousAnimationFrame;
				context.crowdsRequestAnimationNotify = &member.CrowdsRequestAnimationNotify;
				context.memberActive = CrowdsActive;
				context.totalMember = *CrowdsTotal;
			
				AnimationSystems.InitializedAnimationSystems(context);
			}
			
			{
				FFluxPrimeAnimationNotifySystemsContext context;
				context.crowdsDatas = &member;
				context.memberActive = CrowdsActive;
				context.isDebug = false;
				
				AnimationNotifySystems.InitializeAnimationNotifySystems(context);
			}
		}

		if (MovementSystems.IsActive)
		{
			FFluxPrimeMovementSystemsContext context;
			context.isDebug = configurationMovement->IsDebug;
			context.world = World;
			context.currentWaypointCrowds = &member.CrowdsCurrentTarget;
			context.accelerationCrowds = &member.CrowdsAcceleration;
			context.locationCrowds = &member.CrowdsLocation;
			context.rotationCrowds = &member.CrowdsRotation;
			context.maxSpeedCrowds = &member.CrowdsMaxSpeed;
			context.velocityCrowds = &member.CrowdsVelocity;
			context.stateCrowds = &member.CrowdsState;
			context.memberActive = CrowdsActive;
			
			MovementSystems.InitializedMovementSystems(context);
		}

		if (StateMachineSystems.IsActive)
		{
			FFluxPrimeStateMachineSystemsContext context;
			context.isDebug = configurationStateMachine->IsDebug;
			context.stateCrowds = &member.CrowdsState;
			context.abilityRangeCrowds = &member.CrowdsAbilityRange;
			context.locationCrowds = &member.CrowdsLocation;
			context.targetLocationCrowds = &member.CrowdsTarget;
			context.velocityCrowds = &member.CrowdsVelocity;
			context.conditionCrowds = &member.CrowdsCondition;
			context.memberActive = CrowdsActive;
			
			StateMachineSystems.InitializeStateMachineSystems(context);
		}
		
		if (TargetSystems.IsActive)
		{
			TArray<float> aggroDistance;

			for (int i = 0; i < CrowdsCatalog->Num(); ++i)
			{
				aggroDistance.Add((*CrowdsCatalog)[i].CrowdsIdentity->AggroDistance);
			}
			
			FFluxPrimeTargetSystemsContext context;
			context.isDebug = true;
			context.world = World;
			context.locationCrowds = &member.CrowdsLocation;
			context.locationTargetCrowds = &member.CrowdsTarget;
			context.targetCrowdsID = &member.CrowdsTargetID;
			context.idCrowds = &member.CrowdsID;
			context.typeCrowds = &member.CrowdsType;
			context.crowdsTarget = CrowdsTarget;
			context.crowdsAggroDistance = &aggroDistance;
			context.memberActive = CrowdsActive;
			context.onCrowdsManagerActionChange = OnCrowdsManagerActionChange;
			
			TargetSystems.InitializeTargetSystems(context);
		}
		
		{
			TArray<uint16> catalogTotal;

			for (auto& pair : *CrowdsCatalog)
			{
				catalogTotal.Add(pair.CrowdsTotal);
			}
			
			FFluxPrimeCrowdsRenderSystemsContext context;
			context.crowdsComponents = CrowdsComponents;
			context.crowdsCatalogTotal = &catalogTotal;
			context.crowdsCountCatalog = CrowdsCatalog->Num();
			context.locationCrowds = &member.CrowdsLocation;
			context.rotationCrowds = &member.CrowdsRotation;
			context.typeCrowds = &member.CrowdsType;
			context.instanceIndexCrowds = &member.CrowdsID;
			context.memberActive = CrowdsActive;
			
			CrowdsRenderSystems.InitializeRenderSystems(context);
		}
		
		if (HealthSystems.IsActive)
		{
			FFluxPrimeHealthSystemsContext context;
			context.conditionCrowds = &member.CrowdsCondition;
			context.healthCrowds = &member.CrowdsHealth;
			context.memberActive = CrowdsActive;
			context.isDebug = true;
			
			HealthSystems.InitializeHealthSystems(context);
		}
		
		if (AbilitySystems.IsActive)
		{
			TArray<TObjectPtr<UScriptStruct>> crowdsCatalogFragment;
			TArray<FGuid> crowdsCatalogFragmentID;

			for (auto& pair : *CrowdsCatalog)
			{
				FAbilityFragmentWrapper temp = pair.CrowdsIdentity->AbilityFragmentWrapper;
				
				crowdsCatalogFragment.Add(temp.AbilityFragment);
				crowdsCatalogFragmentID.Add(temp.ID);
			}
			
			FFluxPrimeAbilitySystemsContext context;
			context.abilityFragments = configurationAbility->ConfigurationAbilityFragments;
			context.crowdsCountCatalog = CrowdsCatalog->Num();
			context.crowdsCatalogFragment = &crowdsCatalogFragment;
			context.crowdsCatalogFragmentID = &crowdsCatalogFragmentID;
			context.niagaraCallback = NiagaraCallback;
			context.requestAbilityCrowds = &member.CrowdsRequestAbility;
			context.locationCrowds = &member.CrowdsLocation;
			context.rotationCrowds = &member.CrowdsRotation;
			context.damageCrowds = &member.CrowdsDamage;
			context.typeCrowds = &member.CrowdsType;
			context.memberActive = CrowdsActive;
			context.world = World;
			
			AbilitySystems.InitializedAbilitySystems(context);
		}
		
		{
			FFluxPrimeCompactSystemsContext context;
			context.members = CrowdsDatas;
			context.memberActive = CrowdsActive;
			context.crowdsPool = CrowdsPool;
			context.crowdsHeadPool = CrowdsHeadPool;
			context.crowdsLookup = CrowdsLookup;
			context.isDebug = true;
			
			CompactSystems.InitializeCompactSystems(context);
		}
	}
	
	void TickSystems(const float DeltaTime)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Systems_Module);
		
		if (TargetSystems.IsActive) TargetSystems.UpdateTargetSystems();
		if (HealthSystems.IsActive) HealthSystems.UpdateHealthSystems();
		if (SpatialGridSystems.IsActive) SpatialGridSystems.UpdateSpatialGridSystem();
		if (StateMachineSystems.IsActive) StateMachineSystems.UpdateStateMachineSystems();
		if (BoidsSystems.IsActive) BoidsSystems.UpdateBoidsSystems();
		if (GroundHeightSystems.IsActive) GroundHeightSystems.UpdateGroundHeightSystems(DeltaTime);
		if (NavigationSystems.IsActive) NavigationSystems.UpdateNavigationSystems();
		if (MovementSystems.IsActive) MovementSystems.UpdateMovementSystems(DeltaTime);
		if (AnimationSystems.IsActive)
		{
			AnimationSystems.UpdateAnimationSystemsFrame();
			AnimationNotifySystems.UpdateAnimationNotifySystems();
		}
		if (AbilitySystems.IsActive) AbilitySystems.UpdateAbilitySystems();
	
		CrowdsRenderSystems.UpdateRenderCrowdsSystems();
		CompactSystems.UpdateCompactSystems();
	}
	
	void EndPlaySystems()
	{
		if (NavigationSystems.IsActive) NavigationSystems.EndPlayNavigationSystems();
		if (AnimationSystems.IsActive) AnimationSystems.EndPlayAnimationSystems();
	}
};
