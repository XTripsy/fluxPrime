// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "FluxPrimeCrowdsManagerInterface.generated.h"

class UCrowdsIdentity;
// This class does not need to be modified.
UINTERFACE()
class UFluxPrimeCrowdsManagerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class FLUXPRIME_API IFluxPrimeCrowdsManagerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SpawnCrowd(UCrowdsIdentity* identity, FVector location, FRotator rotation);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SwitchAnimationCrowd(UCrowdsIdentity* identity);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlayMontageCrowd(UCrowdsIdentity* identity);
};
