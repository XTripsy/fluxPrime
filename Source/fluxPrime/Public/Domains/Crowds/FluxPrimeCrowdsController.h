// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Core/FluxPrimeBaseController.h"
#include "FluxPrimeCrowdsController.generated.h"

class UFluxPrimeCrowdsManager;
class UFluxPrimeCrowdsDebugComponent;
class UManagerConfiguration;

UCLASS(Blueprintable, HideCategories=(Rendering, Replication, Collision, Input, Activation, AssetUserData, Navigation,
		Actor, LOD, Cooking, Transform, Physics, Networking, LevelInstance, HLOD, WorldPartition, DataLayers))
class FLUXPRIME_API AFluxPrimeCrowdsController : public AFluxPrimeBaseController
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowds", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFluxPrimeCrowdsManager> CrowdsManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowds|Debug", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFluxPrimeCrowdsDebugComponent> CrowdsDebug;
	
public:
	AFluxPrimeCrowdsController();
	
protected:
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	inline UFluxPrimeCrowdsManager* GetCrowdsManager() const
	{
		return CrowdsManager;
	};

	UFUNCTION(BlueprintCallable, BlueprintPure)
	inline UFluxPrimeCrowdsDebugComponent* GetCrowdsDebugComponent() const
	{
		return CrowdsDebug;
	};
};
