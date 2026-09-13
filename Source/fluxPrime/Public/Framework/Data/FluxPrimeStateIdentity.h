// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeStateData.h"
#include "Engine/DataAsset.h"
#include "FluxPrimeStateIdentity.generated.h"

/**
 * 
 */
UCLASS()
class FLUXPRIME_API UFluxPrimeStateIdentity : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FFluxPrimeStateDataEditor> StateData;
};
