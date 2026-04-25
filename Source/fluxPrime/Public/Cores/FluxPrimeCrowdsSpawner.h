// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "FluxPrimeCrowdsSpawner.generated.h"

#define ECC_GroundHeight ECC_GameTraceChannel1

class UCrowdsIdentity;
class IFluxPrimeCrowdsManagerInterface;

UCLASS(NotBlueprintable, HideCategories=(Rendering, Replication, Collision, Input, 
		Actor, LOD, Cooking, Transform, Physics, Networking, LevelInstance, HLOD, WorldPartition, DataLayers))
class FLUXPRIME_API AFluxPrimeCrowdsSpawner final : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TScriptInterface<IFluxPrimeCrowdsManagerInterface> CrowdsManagerInterface;
	
	UPROPERTY(EditAnywhere, Category = "Catalogs", meta = (AllowPrivateAccess = true))
	TArray<TObjectPtr<UCrowdsIdentity>> CrowdsCatalog;

	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (AllowPrivateAccess = true, UIMin = "0.2"))
	float SpawnCrowdPerSeconds = .2f;
	
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (AllowPrivateAccess = true, UIMin = "1.0"))
	int32 TotalSpawnCrowdPerSeconds = 1;
	
	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (AllowPrivateAccess = "true", UIMin = "0.0", UIMax = "50000.0", ClampMin = "0.0"))
	float SpawnMinimalRadius;

	UPROPERTY(EditAnywhere, Category = "Spawner", meta = (AllowPrivateAccess = "true", UIMin = "0.0", UIMax = "50000.0", ClampMin = "0.0"))
	float SpawnMaximalRadius;
	
	FTimerHandle SpawnTimerHandle;
	
private:
	UFUNCTION()
	void Spawning();
	
public:
	AFluxPrimeCrowdsSpawner();
	
	UFUNCTION(BlueprintCallable)
	void StartSpawning();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
