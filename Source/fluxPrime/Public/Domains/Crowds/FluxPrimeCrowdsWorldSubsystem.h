// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Core/FluxPrimeBaseWorldSubsystem.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/WorldSubsystem.h"
#include "FluxPrimeCrowdsWorldSubsystem.generated.h"

class AFluxPrimeCrowdsController;

UCLASS()
class FLUXPRIME_API UFluxPrimeCrowdsWorldSubsystem : public UFluxPrimeBaseWorldSubsystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TWeakObjectPtr<AFluxPrimeCrowdsController> CrowdsController;
	
private:
	AFluxPrimeCrowdsController* GetCrowdsController() const;
	void ExecuteSpawnAction(const FInstancedStruct& payload);
	void ExecuteDamageAction(const FInstancedStruct& payload);
	
protected:
	virtual void HandleWorldInitializedActors(const UWorld::FActorsInitializedParams& Params) override;
	
public:
	UFUNCTION(BlueprintCallable)
	void RequestAction(const FInstancedStruct& payload);
};
