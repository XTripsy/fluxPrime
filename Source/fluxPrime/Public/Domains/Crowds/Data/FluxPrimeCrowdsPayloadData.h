// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeCrowdsData.h"
#include "GameplayTagContainer.h"
#include "FluxPrimeCrowdsPayloadData.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsSpawnPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UFluxPrimeCrowdsIdentity> Identity = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RandomBoundingBox = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag NewState;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalEntityToSpawn;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsShootRayPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StartRayLocation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EndRayLocation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CrowdDamageTaken;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FUniqueNetIdRepl UniqueId; 
};

USTRUCT()
struct FFluxPrimeCrwodsSpawnActionPayload
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsIdentity> Identity;
	
	UPROPERTY()
	TArray<FVector_NetQuantize10> Locations;
	
	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;
	
	UPROPERTY()
	FGameplayTag NewState;
};

