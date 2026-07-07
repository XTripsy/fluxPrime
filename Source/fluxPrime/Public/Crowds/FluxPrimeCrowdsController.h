// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeCrowdsManagerInterface.h"
#include "FluxPrimeCrowdsController.generated.h"

class UFluxPrimeCrowdsManager;
class UFluxPrimeCrowdsNetComponent;
class UManagerConfiguration;

UCLASS(NotBlueprintable, HideCategories=(Rendering, Replication, Collision, Input, 
		Actor, LOD, Cooking, Transform, Physics, Networking, LevelInstance, HLOD, WorldPartition, DataLayers))
class FLUXPRIME_API AFluxPrimeCrowdsController final : public AActor, public IFluxPrimeCrowdsManagerInterface
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CrowdsComponents;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsNetComponent> CrowdsNetComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowds", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFluxPrimeCrowdsManager> CrowdsManager;
	
private:
	UPROPERTY(EditAnywhere, Category = "Crowds | Condition", meta = (AllowPrivateAccess = true))
	bool IsReplicated;
	
public:
	AFluxPrimeCrowdsController();
	
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
