// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeCrowdsManagerInterface.h"
#include "Cores/FluxPrimeStruct.h"
#include "Systems/FluxPrimeBoidsSystems.h"
#include "Systems/FluxPrimeGroundHeightSystems.h"
#include "Systems/FluxPrimeMovementSystems.h"
#include "Systems/FluxPrimeNavigationSystems.h"
#include "Systems/FluxPrimeSpatialGridSystems.h"
#include "FluxPrimeCrowdsManager.generated.h"

UCLASS(NotBlueprintable, HideCategories=(Rendering, Replication, Collision, Input, 
		Actor, LOD, Cooking, Transform, Physics, Networking, LevelInstance, HLOD, WorldPartition, DataLayers))
class FLUXPRIME_API AFluxPrimeCrowdsManager final : public AActor, public IFluxPrimeCrowdsManagerInterface
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CrowdsComponents;
	
private:
	UPROPERTY(EditAnywhere, Category = "Crowds | Catalogs", meta = (AllowPrivateAccess = true))
	TArray<FFluxCatalogCrowds> CatalogCrowds;
	
	UPROPERTY()
	TMap<TObjectPtr<UCrowdsIdentity>, int32> CrowdsTypes;
	
	UPROPERTY()
	int8 CrowdsDataReadIndex = 0;
	
	UPROPERTY()
	TArray<int32> CrowdsDataShortedIndex;
	
	UPROPERTY()
	int32 CrowdsTotal = 0;
	
	UPROPERTY()
	int32 CrowdsActive = 0;
	
	UPROPERTY()
	TArray<int32> GridOffset;
	
	UPROPERTY()
	FFluxPrimeCrowds CrowdsDatas[2];
	
	UPROPERTY()
	FFluxPrimeSpatialGridSystems SpatialGridSystems;
	
	UPROPERTY()
	FFluxPrimeBoidsSystems BoidsSystems;
	
	UPROPERTY()
	FFluxPrimeMovementSystems MovementSystems;
	
	UPROPERTY()
	FFluxPrimeNavigationSystems NavigationSystems;
	
	UPROPERTY()
	FFluxPrimeGroundHeightSystems GroundHeightSystems;
	
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	AActor* targetLocation;
	
public:
	AFluxPrimeCrowdsManager();

private:
	void InitializeComponentCrowds();
	void InitializeCrowds();
	
	void UpdateRenderCrowds();
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
public:
	virtual void SpawnCrowd_Implementation(UCrowdsIdentity* identity, FVector location, FRotator rotation) override;
};
