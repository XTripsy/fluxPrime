// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/Components/FluxPrimeCrowdsSystemsComponent.h"

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
#include "Systems/States/FluxPrimeIdleSystems.h"
#include "Systems/FluxPrimeProxyTargetSystems.h"


UFluxPrimeCrowdsSystemsComponent::UFluxPrimeCrowdsSystemsComponent()
{
	
}

void UFluxPrimeCrowdsSystemsComponent::Initialize(FFluxPrimeCrowdsSystemsComponentContext context)
{
	//CrowdsComponents = context.crowdsComponents;
	ManagerConfiguration = context.managerConfiguration;
	CrowdsRenderSystems = context.crowdsRenderSystems;
	SpatialGridSystems = context.spatialGridSystems;
	BoidsSystems = context.boidsSystems;
	MovementSystems = context.movementSystems;
	NavigationSystems = context.navigationSystems;
	GroundHeightSystems = context.groundHeightSystems;
	AnimationSystems = context.animationSystems;
	ProxyTargetSystems = context.proxyTargetSystems;
	DamageSystems = context.damageSystems;
	CrowdsDatas = context.crowdsDatas;
	CrowdsActive = context.crowdsActive;
	bHasAuthority = context.hasAuthority;
}

void UFluxPrimeCrowdsSystemsComponent::InitializeSystems()
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
	if (SpatialGridSystems->IsActive) SpatialGridSystems->BakeSpatialGridSystems(GetWorld());
	if (GroundHeightSystems->IsActive) GroundHeightSystems->InitializedGroundHeightSystems(configurationGroundHeight->CellSize, configurationGroundHeight->Origin, configurationGroundHeight->CellWidth, configurationGroundHeight->CellHeight);
	if (GroundHeightSystems->IsActive) GroundHeightSystems->BakeGroundHeightSystems(GetWorld());
	if (NavigationSystems->IsActive) NavigationSystems->InitializedNavigationSystems(configurationNavigation->IsDebug, GetWorld());
	if (BoidsSystems->IsActive && SpatialGridSystems->IsActive) BoidsSystems->InitializeBoidsSystems(configurationBoids->SeparationWeight, configurationSpatialGrid->CellSize, configurationSpatialGrid->Origin, configurationSpatialGrid->CellWidth, configurationSpatialGrid->CellHeight);
	if (ProxyTargetSystems->IsActive) ProxyTargetSystems->InitializedProxyTargetSystems(GetWorld(), CrowdsDatas, CrowdsActive, NavigationSystems);
	if (AnimationSystems->IsActive) AnimationSystems->InitializedAnimationSystems(configurationAnimation->IsDebug, CrowdsComponents, 0);
	if (MovementSystems->IsActive) MovementSystems->InitializedMovementSystems(configurationMovement->IsDebug);
	//StateMachineSystems->InitializeStateMachineSystems();
}

/*void UFluxPrimeCrowdsSystemsComponent::UpdateSystems(float DeltaTime)
{
	if (CrowdsComponents.IsEmpty() || CrowdsActive <= 0) return;
	
	if (!HasAuthority() && IsReplicated)
	{
		CrowdsNetComponent->UpdateCrowdsData(NetAcceleration, NetTarget);
		CrowdsNetComponent->UpdateNetData(DeltaTime, GroundHeightSystems, MovementSystems, CrowdsRenderSystems);
		return;
	}
	
	if (SpatialGridSystems.IsActive) SpatialGridSystems.UpdateSpatialGridSystem(GetWorld(), CrowdsDatas, GridOffset, CrowdsDataShortedIndex, CrowdsDataReadIndex, CrowdsActive);
	if (BoidsSystems.IsActive && SpatialGridSystems.IsActive) BoidsSystems.UpdateBoidsSystems(CrowdsDatas[CrowdsDataReadIndex], GridOffset, CrowdsActive);
	if (HasAuthority() && IsReplicated) ShortCrowdsByID();
	if (GroundHeightSystems.IsActive) GroundHeightSystems.UpdateGroundHeightSystems(DeltaTime, CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	if (NavigationSystems.IsActive) NavigationSystems.UpdateNavigationSystems(CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	if (MovementSystems.IsActive) MovementSystems.UpdateMovementSystems(GetWorld(), DeltaTime, CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	if (AnimationSystems.IsActive) AnimationSystems.UpdateAnimationSystemsFrame(GetWorld(), CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	
	StateMachineSystems.UpdateStateMachineSystems(CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	AbilitySystems.UpdateAbilitySystems(CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	IdleSystems.UpdateIdleSystems(CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	
	if (IsShowDebug) ShowDebug();
	
	ForceNetUpdate();
	CrowdsRenderSystems.UpdateRenderCrowdsSystems(CrowdsComponents, CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
}*/

void UFluxPrimeCrowdsSystemsComponent::EndPlaySystems()
{
	if (ProxyTargetSystems->IsActive) ProxyTargetSystems->EndPlayProxyTargetSystems();
	if (NavigationSystems->IsActive) NavigationSystems->EndPlayNavigationSystems();
	if (AnimationSystems->IsActive) AnimationSystems->EndPlayAnimationSystems();
}
