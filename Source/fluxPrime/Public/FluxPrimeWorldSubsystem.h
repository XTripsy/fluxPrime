// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/WorldSubsystem.h"
#include "FluxPrimeWorldSubsystem.generated.h"

class AFluxPrimeCrowdsController;

UCLASS()
class FLUXPRIME_API UFluxPrimeWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TWeakObjectPtr<AFluxPrimeCrowdsController> CrowdsController;
	
private:
	void HandleWorldInitializedActors(const UWorld::FActorsInitializedParams& Params);
	AFluxPrimeCrowdsController* GetCrowdsController() const;
	void ExecuteSpawnAction(const FInstancedStruct& payload);
	void ExecuteStateAction(const FInstancedStruct& payload);
	void ExecuteDamageAction(const FInstancedStruct& payload);
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
public:
	UFUNCTION(BlueprintCallable)
	void RequestAction(const FInstancedStruct& payload);
};
