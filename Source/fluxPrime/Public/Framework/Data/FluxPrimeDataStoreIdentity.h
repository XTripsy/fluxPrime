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
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Is system can run on local client"))
	bool AvailableOnLocalClient;
};

USTRUCT(BlueprintType)
struct FFluxPrimeDataConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type Config")
	UScriptStruct* ExpectedStructType;
};


UCLASS()
class FLUXPRIME_API UFluxPrimeDataStoreIdentity : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FFluxPrimeDataConfigEditor> DataConfigEditors;
};
