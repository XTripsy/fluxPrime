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
	
protected:
	virtual void HandleWorldInitializedActors(const UWorld::FActorsInitializedParams& Params) override;
	
public:
	UFUNCTION(BlueprintCallable)
	void RequestAction(const FInstancedStruct& payload);
	
	/**
	 * Blueprint-facing copy of the kill map. Native callers that query this often should use
	 * UFluxPrimeCrowdsManager::GetPlayerKillsData(), which returns a const reference.
	 */
	UFUNCTION(BlueprintCallable)
	TMap<FUniqueNetIdRepl, int32> GetPlayerKillsData() const;
};
