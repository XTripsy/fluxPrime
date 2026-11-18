#pragma once

#include "CoreMinimal.h"
#include "Detour/DetourNavMesh.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsConfigData.h"
#include "FluxPrimeNavigationData.generated.h"

namespace FluxNavigationConfig
{
	/** Ceiling for the scheduling score so the float priority stays finite and comparable. */
	constexpr float MaxPriorityScore = 1.0e6f;
	/**
	 * Longest accumulated wait used for scheduling, in seconds. Ten minutes keeps every
	 * starvation case ordered without letting the score grow without bound.
	 */
	constexpr float MaxWaitTime = 600.0f;
	/** Points per second of accumulated wait, matching the historical 10x weighting. */
	constexpr float WaitTimeScoreScale = 10.0f;
}

/**
 * Straight-path result for one entity.
 *
 * Only the positions are kept: the poly refs and flags Detour returns alongside them are
 * never read, and carrying them made every copy of this struct 528 bytes instead of 384.
 */
USTRUCT(BlueprintType)
struct FFluxPrimeNavigationWaypoint
{
	GENERATED_BODY()
	
	TStaticArray<FVector, FluxConfig::WaypointsCount> Waypoints;
};

USTRUCT()
struct FFluxPrimeNavigationCorridor
{
	GENERATED_BODY()

	TStaticArray<NavNodeRef, FluxConfig::CorridorsCount> Corridors;
};

struct FFluxPrimeNavigationTask
{
	int16 EntityDataSize;
	int32 EntityDataCellId;
	FVector EntityDataCurrentTargetLocation;
	FVector EntityDataCurrentLocation;
	FVector EntityDataPreviousLocation;
	FVector EntityDataTargetLocation;
	FVector EntityDataLastReplanLocation;
	FVector EntityDataLastMoveLocation;
	float EntityDataWaitTime;
	float EntityDataLastOptimizeTime;
	float EntityDataLastMoveTargetTime;
	FFluxPrimeNavigationCorridor EntityDataCorridors;
	uint8 EntityDataCountCorridor;
	bool EntityDataNeedReplan;
	FFluxPrimeNavigationWaypoint EntityDataWaypoints;
	uint8 EntityDataCountWaypoints;
};

struct FFluxPrimeNavigationDataRequest
{
	/** Scheduling score. Higher wins; see operator<. */
	float PriorityScore = 0.0f;
	int16 DataStoreIndex = 0;
	int32 InternalIndex = 0;
	FFluxPrimeNavigationTask DataTask;

	/** Ascending sort puts the highest priority score first. */
	bool operator<(const FFluxPrimeNavigationDataRequest& Other) const
	{
		return PriorityScore > Other.PriorityScore;
	}
};