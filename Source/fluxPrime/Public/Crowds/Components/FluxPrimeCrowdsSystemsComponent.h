// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Crowds/Modules/FluxPrimeCrowdsSystemsModule.h"
#include "FluxPrimeCrowdsSystemsComponent.generated.h"

class UInstancedStaticMeshComponent;
class UManagerConfiguration;
struct FFluxPrimeCrowdsRenderSystems;
struct FFluxPrimeSpatialGridSystems;
struct FFluxPrimeBoidsSystems;
struct FFluxPrimeMovementSystems;
struct FFluxPrimeNavigationSystems;
struct FFluxPrimeGroundHeightSystems;
struct FFluxPrimeAnimationSystems;
struct FFluxPrimeProxyTargetSystems;
struct FFluxPrimeDamageSystems;
struct FFluxPrimeCrowds;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLUXPRIME_API UFluxPrimeCrowdsSystemsComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CrowdsComponents;
	
	UPROPERTY()
	TObjectPtr<UManagerConfiguration> ManagerConfiguration;
	
	FFluxPrimeCrowdsRenderSystems* CrowdsRenderSystems;
	FFluxPrimeSpatialGridSystems* SpatialGridSystems;
	FFluxPrimeBoidsSystems* BoidsSystems;
	FFluxPrimeMovementSystems* MovementSystems;
	FFluxPrimeNavigationSystems* NavigationSystems;
	FFluxPrimeGroundHeightSystems* GroundHeightSystems;
	FFluxPrimeAnimationSystems* AnimationSystems;
	FFluxPrimeProxyTargetSystems* ProxyTargetSystems;
	FFluxPrimeDamageSystems* DamageSystems;
	
	TStaticArray<FFluxPrimeCrowds, 2>* CrowdsDatas;
	uint16* CrowdsActive;
	
	bool bHasAuthority;
	
public:
	UFluxPrimeCrowdsSystemsComponent();
	void Initialize(FFluxPrimeCrowdsSystemsComponentContext context);
	void InitializeSystems();
	//void UpdateSystems(float DeltaTime);
	void EndPlaySystems();
};
