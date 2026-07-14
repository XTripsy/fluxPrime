#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Crowds/ManagerConfiguration/ManagerConfiguration.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationAnimationSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationBoidsSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationDamageSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationGroundHeightSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationMovementSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationNavigationSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationProxyTargetSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationSpatialGridSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationStateMachineSystems.h"
#include "Systems/FluxPrimeBoidsSystems.h"
#include "Systems/FluxPrimeGroundHeightSystems.h"
#include "Systems/FluxPrimeMovementSystems.h"
#include "Systems/FluxPrimeNavigationSystems.h"
#include "Systems/FluxPrimeSpatialGridSystems.h"
#include "Systems/FluxPrimeAnimationSystems.h"
#include "Systems/FluxPrimeCompactSystems.h"
#include "Systems/FluxPrimeCrowdsRenderSystems.h"
#include "Systems/FluxPrimeDamageSystems.h"
#include "Systems/FluxPrimeHealthSystems.h"
#include "Systems/FluxPrimeStateMachineSystems.h"
#include "Systems/FluxPrimeTargetSystems.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Systems/FluxPrimeAnimationNotifySystems.h"
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
	TMap<FFluxPrimeCrowdsLookup, int32>* crowdsLookup = nullptr;
	
	bool hasAuthority;
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
	
	FFluxPrimeCrowdsRenderSystems CrowdsRenderSystems;
	FFluxPrimeSpatialGridSystems SpatialGridSystems;
	FFluxPrimeBoidsSystems BoidsSystems;
	FFluxPrimeMovementSystems MovementSystems;
	FFluxPrimeNavigationSystems NavigationSystems;
	FFluxPrimeGroundHeightSystems GroundHeightSystems;
	FFluxPrimeAnimationSystems AnimationSystems;
	FFluxPrimeAnimationNotifySystems AnimationNotifySystems;
	FFluxPrimeDamageSystems DamageSystems;
	FFluxPrimeStateMachineSystems StateMachineSystems;
	FFluxPrimeHealthSystems HealthSystems;
	FFluxPrimeCompactSystems CompactSystems;
	
	FFluxPrimeCrowds* CrowdsDatas = nullptr;
	uint16* CrowdsActive = nullptr;
	uint16* CrowdsTotal = nullptr;
	TMap<FFluxPrimeCrowdsLookup, int32>* CrowdsLookup = nullptr;
	
	bool bHasAuthority = false;
	
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
	UPROPERTY()
	FFluxPrimeTargetSystems TargetSystems;
	
public:
	void Damage(uint32 index)
	{
		HealthSystems.OnHitHealthSystems(index);
	}
	
	void Initialize(FFluxPrimeCrowdsSystemsComponentContext context)
	{
		check(context.world);
		check(context.crowdsDatas);
		check(context.crowdsActive);
		check(context.crowdsLookup);
		check(context.crowdsCatalog);
		check(context.crowdsAnimationSoftRef);
		
		CrowdsComponents = context.crowdsComponents;
		ManagerConfiguration = context.managerConfiguration;
		CrowdsCatalog = context.crowdsCatalog;
		CrowdsAnimationSoftRef = context.crowdsAnimationSoftRef;
		
		CrowdsDatas = context.crowdsDatas;
		CrowdsActive = context.crowdsActive;
		CrowdsTotal = context.crowdsTotal;
		CrowdsLookup = context.crowdsLookup;
		
		bHasAuthority = context.hasAuthority;
		
		World = context.world;
	}

	void InitializeSystems()
	{
		const FFluxPrimeConfigurationGroundHeightSystems* configurationGroundHeight = nullptr;
		const FFluxPrimeConfigurationSpatialGridSystems* configurationSpatialGrid = nullptr;
		const FFluxPrimeConfigurationNavigationSystems* configurationNavigation = nullptr;
		const FFluxPrimeConfigurationBoidsSystems* configurationBoids = nullptr;
		const FFluxPrimeConfigurationMovementSystems* configurationMovement = nullptr;
		const FFluxPrimeConfigurationAnimationSystems* configurationAnimation = nullptr;
		const FFluxPrimeConfigurationProxyTargetSystems* configurationProxyTarget = nullptr;
		const FFluxPrimeConfigurationDamageSystems* configurationDamage = nullptr;
		const FFluxPrimeConfigurationStateMachineSystems* configurationStateMachine = nullptr;
		
		for (auto& pair : ManagerConfiguration->ConfigurationFragments)
		{
			if (!pair.IsValid()) continue;
			
			if (const FFluxPrimeConfigurationGroundHeightSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationGroundHeightSystems>())
			{
				GroundHeightSystems.IsActive = true;
				configurationGroundHeight = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationSpatialGridSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationSpatialGridSystems>())
			{
				if (!bHasAuthority) continue;
				configurationSpatialGrid = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationNavigationSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationNavigationSystems>())
			{
				if (!bHasAuthority) continue;
				NavigationSystems.IsActive = true;
				configurationNavigation = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationBoidsSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationBoidsSystems>())
			{
				if (!bHasAuthority) continue;
				BoidsSystems.IsActive = true;
				configurationBoids = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationMovementSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationMovementSystems>())
			{
				MovementSystems.IsActive = true;
				configurationMovement = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationAnimationSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationAnimationSystems>())
			{
				if (!bHasAuthority) continue;
				AnimationSystems.IsActive = true;
				configurationAnimation = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationDamageSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationDamageSystems>())
			{
				if (!bHasAuthority) continue;
				DamageSystems.IsActive = true;
				configurationDamage = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationStateMachineSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationStateMachineSystems>())
			{
				StateMachineSystems.IsActive = true;
				configurationStateMachine = configuration;
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
		
		// ground height
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
			context.locationCrowds = &member.CrowdsLocation;
			context.crowdsCellID = &member.CrowdsCellID;
			context.navigationPathCrowds = &member.CrowdsNavigationPath;
			context.locationTargetCrowds = &member.CrowdsTargetLocation;
			context.indexNavigationPathCrowds = &member.CrowdsIndexNavigationPath;
			context.totalNavigationPathCrowds = &member.CrowdsTotalNavigationPath;
			context.requestNavigationPathCrowds = &member.CrowdsRequestNavigationPath;
			context.locationCurrentTargetCrowds = &member.CrowdsCurrentTargetLocationPath;
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
				context.crowdsCatalog = CrowdsCatalog;
				context.crowdsAnimationSoftRef = CrowdsAnimationSoftRef;
				context.world = World;
				context.crowdsCellID = &member.CrowdsCellID;
				context.crowdsCurrentTargetLocationPath = &member.CrowdsCurrentTargetLocationPath;
				context.idCrowds = &member.CrowdsID;
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
			context.currentPathCrowds = &member.CrowdsCurrentTargetLocationPath;
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
			context.locationCrowds = &member.CrowdsLocation;
			context.targetLocationCrowds = &member.CrowdsTargetLocation;
			context.velocityCrowds = &member.CrowdsVelocity;
			context.conditionCrowds = &member.CrowdsCondition;
			context.memberActive = CrowdsActive;
			
			StateMachineSystems.InitializeStateMachineSystems(context);
		}
		
		{
			FFluxPrimeTargetSystemsContext context;
			context.isDebug = true;
			context.world = World;
			context.locationCrowds = &member.CrowdsLocation;
			context.locationTargetCrowds = &member.CrowdsTargetLocation;
			context.requestNavigationPathCrowds = &member.CrowdsRequestNavigationPath;
			context.memberActive = CrowdsActive;
			
			TargetSystems.InitializeTargetSystems(context);
		}
		
		{
			FFluxPrimeCrowdsRenderSystemsContext context;
			context.crowdsComponents = CrowdsComponents;
			context.crowdsCatalog = CrowdsCatalog;
			context.locationCrowds = &member.CrowdsLocation;
			context.rotationCrowds = &member.CrowdsRotation;
			context.typeCrowds = &member.CrowdsType;
			context.memberActive = CrowdsActive;
			
			CrowdsRenderSystems.InitializeRenderSystems(context);
		}
		
		{
			FFluxPrimeHealthSystemsContext context;
			context.conditionCrowds = &member.CrowdsCondition;
			context.healthCrowds = &member.CrowdsHealth;
			context.memberActive = CrowdsActive;
			context.isDebug = true;
			
			HealthSystems.InitializeHealthSystems(context);
		}
		
		{
			FFluxPrimeCompactSystemsContext context;
			context.members = CrowdsDatas;
			context.memberActive = CrowdsActive;
			context.crowdsLookup = CrowdsLookup;
			context.isDebug = true;
			
			CompactSystems.InitializeCompactSystems(context);
		}
	}
	
	void TickSystems(const float DeltaTime)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Systems_Module);
		
		HealthSystems.UpdateHealthSystems();
		if (SpatialGridSystems.IsActive) SpatialGridSystems.UpdateSpatialGridSystem();
		if (StateMachineSystems.IsActive) StateMachineSystems.UpdateStateMachineSystems();
		if (BoidsSystems.IsActive) BoidsSystems.UpdateBoidsSystems();
		if (GroundHeightSystems.IsActive) GroundHeightSystems.UpdateGroundHeightSystems(DeltaTime);
		TargetSystems.UpdateTargetSystems();
		if (NavigationSystems.IsActive) NavigationSystems.UpdateNavigationSystems();
		if (MovementSystems.IsActive) MovementSystems.UpdateMovementSystems(DeltaTime);
		if (AnimationSystems.IsActive)
		{
			AnimationSystems.UpdateAnimationSystemsFrame();
			AnimationNotifySystems.UpdateAnimationNotifySystems();
		}
	
		CrowdsRenderSystems.UpdateRenderCrowdsSystems();
		CompactSystems.UpdateCompactSystems();
	}
	
	void EndPlaySystems()
	{
		if (NavigationSystems.IsActive) NavigationSystems.EndPlayNavigationSystems();
		if (AnimationSystems.IsActive) AnimationSystems.EndPlayAnimationSystems();
	}
};
