// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeStruct.h"
#include "FluxPrimePayload.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeSpawnPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UCrowdsIdentity> Identity = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;
};

USTRUCT(BlueprintType)
struct FFluxPrimeStatePayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFluxPrimeCrowdState NewState = EFluxPrimeCrowdState::StateIdle;
};

USTRUCT(BlueprintType)
struct FFluxPrimeDamagePayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InstanceID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ISMC;
};
