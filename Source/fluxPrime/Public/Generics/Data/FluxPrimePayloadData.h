// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimePayloadData.generated.h"

USTRUCT()
struct FFluxPrimeSnapshotData
{
	GENERATED_BODY()
	
	UPROPERTY()
	int16 CrowdID = INDEX_NONE;
	
	UPROPERTY()
	int8 CrowdType = INDEX_NONE;
	
	UPROPERTY()
	int8 CrowdState = INDEX_NONE;
	
	UPROPERTY()
	FVector_NetQuantize10 CrowdNewWaypointLocation = FVector::ZeroVector;
};

USTRUCT()
struct FFluxPrimeSnapshotDataPayload
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<FFluxPrimeSnapshotData> SnapshotDatas;
};