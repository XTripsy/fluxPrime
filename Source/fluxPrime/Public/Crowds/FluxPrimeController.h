// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeController.generated.h"

class UFluxPrimeCrowdsManager;
class UFluxPrimeCrowdsNetManager;
class UManagerConfiguration;

UCLASS(NotBlueprintable, HideCategories=(Rendering, Replication, Collision, Input, 
		Actor, LOD, Cooking, Transform, Physics, Networking, LevelInstance, HLOD, WorldPartition, DataLayers))
class FLUXPRIME_API AFluxPrimeController final : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CrowdsComponents;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsNetManager> CrowdsNetComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowds", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFluxPrimeCrowdsManager> CrowdsManager;
	
private:
	UPROPERTY(EditAnywhere, Category = "Crowds | Condition", meta = (AllowPrivateAccess = true))
	bool IsReplicated;
	
public:
	AFluxPrimeController();
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	inline UFluxPrimeCrowdsManager* GetCrowdsManager() const
	{
		ensure(CrowdsManager);
		return CrowdsManager;
	};
};
