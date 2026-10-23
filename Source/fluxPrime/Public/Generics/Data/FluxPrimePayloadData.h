// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimePayloadData.generated.h"

USTRUCT()
struct FFluxPrimeCrowdsSnapshotData
{
	GENERATED_BODY()
	
	UPROPERTY()
	int16 CrowdIndex = INDEX_NONE;
	
	UPROPERTY()
	int8 CrowdType = INDEX_NONE;
	
	UPROPERTY()
	int8 CrowdState = INDEX_NONE;
	
	UPROPERTY()
	bool CrowdConditionAlive = false;
	
	UPROPERTY()
	FVector_NetQuantize10 CrowdNewWaypointLocation = FVector::ZeroVector;
};

USTRUCT()
struct FFluxPrimeCrowdsSnapshotDataPayload
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<FFluxPrimeCrowdsSnapshotData> SnapshotDatas;
};

USTRUCT()
struct FFluxPrimeCrowdsSnapshotActiveEntityPayload
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<int32> SnapshotActiveEntityCountDatas;
};

USTRUCT()
struct FFluxPrimeCrowdsKillDataPayload
{
	GENERATED_BODY()
	
	UPROPERTY()
	FUniqueNetIdRepl PlayerId;
	
	UPROPERTY()
	int32 PlayerKillCount = INDEX_NONE;
};
