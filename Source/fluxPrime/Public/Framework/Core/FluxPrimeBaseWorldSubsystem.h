// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FluxPrimeBaseWorldSubsystem.generated.h"

class AFluxPrimeBaseController;
/**
 * 
 */
UCLASS(Abstract)
class FLUXPRIME_API UFluxPrimeBaseWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
protected:
	UPROPERTY()
	TWeakObjectPtr<AFluxPrimeBaseController> Controller;
	
protected:
	AFluxPrimeBaseController* GetBaseController() const;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
protected:
	virtual void HandleWorldInitializedActors(const UWorld::FActorsInitializedParams& Params);
};
