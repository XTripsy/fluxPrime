// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "FluxPrimeCrowdsManagerInterface.h"
#include "Cores/FluxPrimeStruct.h"
#include "Systems/FluxPrimeBoidsSystems.h"
#include "Systems/FluxPrimeGroundHeightSystems.h"
#include "Systems/FluxPrimeMovementSystems.h"
#include "Systems/FluxPrimeNavigationSystems.h"
#include "Systems/FluxPrimeSpatialGridSystems.h"
#include "Systems/FluxPrimeAnimationSystems.h"
#include "Systems/FluxPrimeDamageSystems.h"
#include "Systems/FluxPrimeProxyTargetSystems.h"
#include "FluxPrimeCrowdsManager.generated.h"

class UManagerConfiguration;

UCLASS(NotBlueprintable, HideCategories=(Rendering, Replication, Collision, Input, 
		Actor, LOD, Cooking, Transform, Physics, Networking, LevelInstance, HLOD, WorldPartition, DataLayers))
class FLUXPRIME_API AFluxPrimeCrowdsManager final : public AActor, public IFluxPrimeCrowdsManagerInterface
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CrowdsComponents;
	
private:
	UPROPERTY(EditAnywhere, Category = "Crowds | Condition", meta = (AllowPrivateAccess = true))
	bool IsReplicated;
	
	UPROPERTY(EditAnywhere, Category = "Crowds | Condition", meta = (AllowPrivateAccess = true))
	bool IsShowDebug;
	
	UPROPERTY(EditAnywhere, Category = "Crowds | Configuration", meta = (AllowPrivateAccess = true))
	TObjectPtr<UManagerConfiguration> ManagerConfiguration;
	
	UPROPERTY(EditAnywhere, Category = "Crowds | Catalogs", meta = (AllowPrivateAccess = true))
	TArray<FFluxCatalogCrowds> CrowdsCatalog;
	
	UPROPERTY(EditDefaultsOnly, Category = "Crowds | Data")
	TMap<FName, TSoftObjectPtr<UStaticMesh>> CrowdsMeshSoftRef;
	
	UPROPERTY(EditDefaultsOnly, Category = "Crowds | Data")
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>> CrowdsAnimationSoftRef;
	
	TSharedPtr<FStreamableHandle> StreamingHandle;
	
	UPROPERTY()
	TMap<FName, int32> CrowdsTypes;
	
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
	
	TStaticArray<FFluxPrimeCrowds, 2> CrowdsDatas;
	
	TSharedPtr<FFluxPrimeSpatialGridSystems> SpatialGridSystems = nullptr;
	TSharedPtr<FFluxPrimeBoidsSystems> BoidsSystems = nullptr;
	TSharedPtr<FFluxPrimeMovementSystems> MovementSystems = nullptr;
	TSharedPtr<FFluxPrimeNavigationSystems> NavigationSystems = nullptr;
	TSharedPtr<FFluxPrimeGroundHeightSystems> GroundHeightSystems = nullptr;
	TSharedPtr<FFluxPrimeAnimationSystems> AnimationSystems = nullptr;
	TSharedPtr<FFluxPrimeProxyTargetSystems> ProxyTargetSystems = nullptr;
	TSharedPtr<FFluxPrimeDamageSystems> DamageSystems = nullptr;
	
	/*UPROPERTY()
	FFluxPrimeSpatialGridSystems SpatialGridSystems;
	
	UPROPERTY()
	FFluxPrimeBoidsSystems BoidsSystems;
	
	UPROPERTY()
	FFluxPrimeMovementSystems MovementSystems;
	
	UPROPERTY()
	FFluxPrimeNavigationSystems NavigationSystems;
	
	UPROPERTY()
	FFluxPrimeGroundHeightSystems GroundHeightSystems;
	
	UPROPERTY()
	FFluxPrimeAnimationSystems AnimationSystems; 
	
	UPROPERTY()
	FFluxPrimeProxyTargetSystems ProxyTargetSystems;
	
	UPROPERTY()
	FFluxPrimeDamageSystems DamageSystems;*/
	
public:
	AFluxPrimeCrowdsManager();

private:
	void ShowDebug();
	
	void PreLoading();
	
	UFUNCTION()
	void InitializeSystems();
	
	void InitializeComponentCrowds();
	void InitializeCrowds();
	
	void UpdateRenderCrowds();
	
	UFUNCTION()
	void OnAttackNotify(int32 memberID);
	
	UFUNCTION()
	void OnSpawnSFXNotify(int32 memberID);
	
	UFUNCTION()
	void OnSpawnVFXNotify(int32 memberID);
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitProperties() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	virtual void SpawnCrowd_Implementation(UCrowdsIdentity* identity, FVector location, FRotator rotation) override;
	virtual void SwitchAnimationCrowd_Implementation(UCrowdsIdentity* identity) override;
	virtual void PlayMontageCrowd_Implementation(UCrowdsIdentity* identity) override;
	virtual void TakeDamage_Implementation(UCrowdsIdentity* Identity) override;
};
