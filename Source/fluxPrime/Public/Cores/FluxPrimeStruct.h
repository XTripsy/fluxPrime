// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeStruct.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeNet
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector_NetQuantize100 NetLocation = FVector_NetQuantize100::Zero();
	
	UPROPERTY(EditAnywhere)
	int8 NetRotation = 0;
	
	UPROPERTY(EditAnywhere)
	int8 NetType;
	
	UPROPERTY(EditAnywhere)
	int8 NetID;
	
	UPROPERTY(EditAnywhere)
	bool isActive = true;
	
};
