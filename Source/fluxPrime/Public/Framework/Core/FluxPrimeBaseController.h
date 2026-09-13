// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseController.generated.h"

class UFluxPrimeNetManager;

UCLASS(Abstract, NotBlueprintable, HideCategories=(Rendering, Replication, Collision, Input, 
			Actor, LOD, Cooking, Transform, Physics, Networking, LevelInstance, HLOD, WorldPartition, DataLayers))
class FLUXPRIME_API AFluxPrimeBaseController : public AActor
{
	GENERATED_BODY()
		
private:
	UPROPERTY(EditAnywhere, Category = "Controller | Net", meta = (AllowPrivateAccess = true))
	bool IsReplicated;
		
protected:
	UPROPERTY()
	TObjectPtr<UFluxPrimeNetManager> NetManager;
		
public:
	AFluxPrimeBaseController();
		
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;
		
public:
	TObjectPtr<UFluxPrimeNetManager> GetNetManager();
};
