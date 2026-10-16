// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SystemsConfiguration.generated.h"

/**
 * 
 */
UCLASS()
class FLUXPRIME_API USystemsConfiguration : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, meta=(BaseStruct="FluxPrimeBaseModule", ExcludeBaseStruct))
	TArray<FInstancedStruct> SystemFragments;
};
