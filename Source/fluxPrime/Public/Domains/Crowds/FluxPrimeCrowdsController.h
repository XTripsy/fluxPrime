// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Core/FluxPrimeBaseController.h"
#include "FluxPrimeCrowdsController.generated.h"

class UFluxPrimeCrowdsManager;
class UManagerConfiguration;

UCLASS(NotBlueprintable, HideCategories=(Rendering, Replication, Collision, Input, 
		Actor, LOD, Cooking, Transform, Physics, Networking, LevelInstance, HLOD, WorldPartition, DataLayers))
class FLUXPRIME_API AFluxPrimeCrowdsController final : public AFluxPrimeBaseController
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowds", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFluxPrimeCrowdsManager> CrowdsManager;
	
public:
	AFluxPrimeCrowdsController();
	
protected:
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	inline UFluxPrimeCrowdsManager* GetCrowdsManager() const
	{
		ensure(CrowdsManager);
		return CrowdsManager;
	};
};
