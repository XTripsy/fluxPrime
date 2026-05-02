// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StructUtils/InstancedStruct.h"
#include "ManagerConfiguration.generated.h"

UCLASS(BlueprintType)
class FLUXPRIME_API UManagerConfiguration : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, meta=(BaseStruct="FluxPrimeBaseConfiguration", ExcludeBaseStruct))
	TArray<FInstancedStruct> ConfigurationFragments;
};
