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
#include "Systems/FluxPrimeBoidsSystems.h"
#include "Systems/FluxPrimeGroundHeightSystems.h"
#include "Systems/FluxPrimeMovementSystems.h"
#include "Systems/FluxPrimeNavigationSystems.h"
#include "Systems/FluxPrimeSpatialGridSystems.h"
#include "Systems/FluxPrimeAnimationSystems.h"
#include "Systems/FluxPrimeCrowdsRenderSystems.h"
#include "Systems/FluxPrimeDamageSystems.h"
#include "Systems/FluxPrimeProxyTargetSystems.h"
#include "Systems/FluxPrimeStateMachineSystems.h"
#include "FluxPrimeCrowdsSystemsModule.generated.h"

USTRUCT()
struct FFluxPrimeCrowdsSystemsComponentContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UManagerConfiguration> managerConfiguration;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* crowdsComponents = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	TArray<int32>* gridOffset = nullptr;
	TArray<int32>* crowdsDataShortedIndex = nullptr;
	
	int8* crowdsDataReadIndex = nullptr;
	
	FFluxPrimeCrowdsRenderSystems* crowdsRenderSystems = nullptr;
	FFluxPrimeSpatialGridSystems* spatialGridSystems = nullptr;
	FFluxPrimeBoidsSystems* boidsSystems = nullptr;
	FFluxPrimeMovementSystems* movementSystems = nullptr;
	FFluxPrimeNavigationSystems* navigationSystems = nullptr;
	FFluxPrimeGroundHeightSystems* groundHeightSystems = nullptr;
	FFluxPrimeAnimationSystems* animationSystems = nullptr;
	FFluxPrimeProxyTargetSystems* proxyTargetSystems = nullptr;
	FFluxPrimeDamageSystems* damageSystems = nullptr;
	FFluxPrimeStateMachineSystems* stateMachineSystems = nullptr;
	
	TStaticArray<FFluxPrimeCrowds, 2>* crowdsDatas = nullptr;
	uint16* crowdsActive = nullptr;
	uint16* crowdsTotal = nullptr;
	
	bool hasAuthority;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsSystemsModule
{
	GENERATED_BODY()
	
private:
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* CrowdsComponents = nullptr;
	
	UPROPERTY()
	TObjectPtr<UManagerConfiguration> ManagerConfiguration;
	
	TArray<int32>* GridOffset = nullptr;
	TArray<int32>* CrowdsDataShortedIndex = nullptr;
	int8* CrowdsDataReadIndex = nullptr;
	
	FFluxPrimeCrowdsRenderSystems* CrowdsRenderSystems = nullptr;
	FFluxPrimeSpatialGridSystems* SpatialGridSystems = nullptr;
	FFluxPrimeBoidsSystems* BoidsSystems = nullptr;
	FFluxPrimeMovementSystems* MovementSystems = nullptr;
	FFluxPrimeNavigationSystems* NavigationSystems = nullptr;
	FFluxPrimeGroundHeightSystems* GroundHeightSystems = nullptr;
	FFluxPrimeAnimationSystems* AnimationSystems = nullptr;
	FFluxPrimeProxyTargetSystems* ProxyTargetSystems = nullptr;
	FFluxPrimeDamageSystems* DamageSystems = nullptr;
	FFluxPrimeStateMachineSystems* StateMachineSystems = nullptr;
	
	TStaticArray<FFluxPrimeCrowds, 2>* CrowdsDatas = nullptr;
	uint16* CrowdsActive = nullptr;
	uint16* CrowdsTotal = nullptr;
	
	bool bHasAuthority = false;
	
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
public:
	void Initialize(FFluxPrimeCrowdsSystemsComponentContext context)
	{
		check(context.world);
		check(context.crowdsDatas);
		check(context.crowdsActive);
		check(context.crowdsDataReadIndex);
		
		CrowdsComponents = context.crowdsComponents;
		ManagerConfiguration = context.managerConfiguration;
		
		GridOffset = context.gridOffset;
		CrowdsDataShortedIndex = context.crowdsDataShortedIndex;
		CrowdsDataReadIndex = context.crowdsDataReadIndex;
		
		CrowdsRenderSystems = context.crowdsRenderSystems;
		SpatialGridSystems = context.spatialGridSystems;
		BoidsSystems = context.boidsSystems;
		MovementSystems = context.movementSystems;
		NavigationSystems = context.navigationSystems;
		GroundHeightSystems = context.groundHeightSystems;
		AnimationSystems = context.animationSystems;
		ProxyTargetSystems = context.proxyTargetSystems;
		DamageSystems = context.damageSystems;
		StateMachineSystems = context.stateMachineSystems;
		
		CrowdsDatas = context.crowdsDatas;
		CrowdsActive = context.crowdsActive;
		CrowdsTotal = context.crowdsTotal;
		
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
		
		for (auto& pair : ManagerConfiguration->ConfigurationFragments)
		{
			if (!pair.IsValid()) continue;
			
			if (const FFluxPrimeConfigurationGroundHeightSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationGroundHeightSystems>())
			{
				GroundHeightSystems->IsActive = true;
				configurationGroundHeight = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationSpatialGridSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationSpatialGridSystems>())
			{
				//if (!HasAuthority()) continue;
				if (!bHasAuthority) continue;
				SpatialGridSystems->IsActive = true;
				configurationSpatialGrid = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationNavigationSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationNavigationSystems>())
			{
				//if (!HasAuthority()) continue;
				if (!bHasAuthority) continue;
				NavigationSystems->IsActive = true;
				configurationNavigation = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationBoidsSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationBoidsSystems>())
			{
				//if (!HasAuthority()) continue;
				if (!bHasAuthority) continue;
				BoidsSystems->IsActive = true;
				configurationBoids = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationMovementSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationMovementSystems>())
			{
				MovementSystems->IsActive = true;
				configurationMovement = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationAnimationSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationAnimationSystems>())
			{
				//if (!HasAuthority()) continue;
				if (!bHasAuthority) continue;
				AnimationSystems->IsActive = true;
				configurationAnimation = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationProxyTargetSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationProxyTargetSystems>())
			{
				//if (!HasAuthority()) continue;
				if (!bHasAuthority) continue;
				ProxyTargetSystems->IsActive = true;
				configurationProxyTarget = configuration;
				continue;
			}
			
			if (const FFluxPrimeConfigurationDamageSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationDamageSystems>())
			{
				//if (!HasAuthority()) continue;
				if (!bHasAuthority) continue;
				DamageSystems->IsActive = true;
				configurationDamage = configuration;
				continue;
			}
		}
		
	#if !WITH_EDITOR
		IsShowDebug = false;
		if (configurationSpatialGrid) configurationSpatialGrid->IsDebug = false;
		if (configurationNavigation) configurationNavigation->IsDebug = false;
		if (configurationAnimation) configurationAnimation->IsDebug = false;
		if (configurationMovement) configurationMovement->IsDebug = false;
	#endif
		
		if (SpatialGridSystems->IsActive) SpatialGridSystems->InitializedSpatialGridSystems(configurationSpatialGrid->IsDebug, configurationSpatialGrid->CellSize, configurationSpatialGrid->Origin, configurationSpatialGrid->CellWidth, configurationSpatialGrid->CellHeight);
		if (SpatialGridSystems->IsActive) SpatialGridSystems->BakeSpatialGridSystems(World);
		if (GroundHeightSystems->IsActive) GroundHeightSystems->InitializedGroundHeightSystems(configurationGroundHeight->CellSize, configurationGroundHeight->Origin, configurationGroundHeight->CellWidth, configurationGroundHeight->CellHeight);
		if (GroundHeightSystems->IsActive) GroundHeightSystems->BakeGroundHeightSystems(World);
		if (NavigationSystems->IsActive) NavigationSystems->InitializedNavigationSystems(configurationNavigation->IsDebug, World);
		if (BoidsSystems->IsActive && SpatialGridSystems->IsActive) BoidsSystems->InitializeBoidsSystems(configurationBoids->SeparationWeight, configurationSpatialGrid->CellSize, configurationSpatialGrid->Origin, configurationSpatialGrid->CellWidth, configurationSpatialGrid->CellHeight);
		if (ProxyTargetSystems->IsActive) ProxyTargetSystems->InitializedProxyTargetSystems(World, CrowdsDatas, CrowdsActive, NavigationSystems);
		if (AnimationSystems->IsActive) AnimationSystems->InitializedAnimationSystems(configurationAnimation->IsDebug, *CrowdsComponents, *CrowdsTotal);
		if (MovementSystems->IsActive) MovementSystems->InitializedMovementSystems(configurationMovement->IsDebug);
		StateMachineSystems->InitializeStateMachineSystems();
	}
	
	void TickSystems(float DeltaTime)
	{
		if (CrowdsComponents->IsEmpty() || *CrowdsActive <= 0) return;
	
		if (SpatialGridSystems->IsActive) SpatialGridSystems->UpdateSpatialGridSystem(World, *CrowdsDatas, *GridOffset, *CrowdsDataShortedIndex, *CrowdsDataReadIndex, *CrowdsActive);
		StateMachineSystems->UpdateStateMachineSystems((*CrowdsDatas)[*CrowdsDataReadIndex], *CrowdsActive);
		if (BoidsSystems->IsActive && SpatialGridSystems->IsActive) BoidsSystems->UpdateBoidsSystems((*CrowdsDatas)[*CrowdsDataReadIndex], *GridOffset, *CrowdsActive);
		if (GroundHeightSystems->IsActive) GroundHeightSystems->UpdateGroundHeightSystems(DeltaTime, (*CrowdsDatas)[*CrowdsDataReadIndex], *CrowdsActive);
		if (NavigationSystems->IsActive) NavigationSystems->UpdateNavigationSystems((*CrowdsDatas)[*CrowdsDataReadIndex], *CrowdsActive);
		if (MovementSystems->IsActive) MovementSystems->UpdateMovementSystems(World, DeltaTime, (*CrowdsDatas)[*CrowdsDataReadIndex], *CrowdsActive);
		if (AnimationSystems->IsActive) AnimationSystems->UpdateAnimationSystemsFrame(World, (*CrowdsDatas)[*CrowdsDataReadIndex], *CrowdsActive);
	
		CrowdsRenderSystems->UpdateRenderCrowdsSystems((*CrowdsComponents), (*CrowdsDatas)[*CrowdsDataReadIndex], *CrowdsActive);
	}
	
	void EndPlaySystems()
	{
		if (ProxyTargetSystems->IsActive) ProxyTargetSystems->EndPlayProxyTargetSystems();
		if (NavigationSystems->IsActive) NavigationSystems->EndPlayNavigationSystems();
		if (AnimationSystems->IsActive) AnimationSystems->EndPlayAnimationSystems();
	}
};
