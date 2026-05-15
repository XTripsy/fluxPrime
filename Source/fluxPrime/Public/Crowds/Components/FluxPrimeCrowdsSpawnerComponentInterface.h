// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FluxPrimeCrowdsSpawnerComponentInterface.generated.h"

class UCrowdsIdentity;

UINTERFACE()
class UFluxPrimeCrowdsSpawnerComponentInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FLUXPRIME_API IFluxPrimeCrowdsSpawnerComponentInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SpawnCrowd(UCrowdsIdentity* identity, FVector location, FRotator rotation);
};
