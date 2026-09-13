// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "FluxPrimeDataStoreIdentity.generated.h"

USTRUCT()
struct FFluxPrimeDataConfigEditor 
{
	GENERATED_BODY()
    
	UPROPERTY(EditAnywhere)
	FGameplayTag DataTag;
    
	UPROPERTY(EditAnywhere)
	UScriptStruct* StructType;
};

UCLASS()
class FLUXPRIME_API UFluxPrimeDataStoreIdentity : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FFluxPrimeDataConfigEditor> DataConfigEditors;
};
