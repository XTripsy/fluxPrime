// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeCrowdsData.h"
#include "GameplayTagContainer.h"
#include "FluxPrimeCrowdsPayloadData.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeSpawnPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UFluxPrimeCrowdsIdentity> Identity = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag NewState;
};

USTRUCT(BlueprintType)
struct FFluxPrimeDamagePayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InstanceID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ISMC;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CrowdDamageTaken;
};

USTRUCT()
struct FFluxPrimeSpawnActionPayload
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsIdentity> Identity;
	
	UPROPERTY()
	FVector Location;
	
	UPROPERTY()
	FRotator Rotation;
	
	UPROPERTY()
	FGameplayTag NewState;
};

USTRUCT()
struct FFluxPrimeDamageActionPayload
{
	GENERATED_BODY()
	
	UPROPERTY()
	int16 CrowdID;
	
	UPROPERTY()
	int8 CrowdType;
	
	UPROPERTY()
	int16 CrowdDamageTaken;
};

USTRUCT()
struct FFluxPrimeChangeTargetActionPayload
{
	GENERATED_BODY()
	
	UPROPERTY()
	int16 CrowdID;
	
	UPROPERTY()
	int8 CrowdType;
	
	UPROPERTY()
	uint16 TargetID;
	
	UPROPERTY()
	FVector NewTargetLocation;
};

USTRUCT()
struct FFluxPrimeRegisterTargetActionPayload
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> NewCrowdsTarget;
};
