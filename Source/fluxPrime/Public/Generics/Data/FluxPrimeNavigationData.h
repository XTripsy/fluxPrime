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

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsWaypoint
{
	GENERATED_BODY()
	
	TStaticArray<FVector, FluxConfig::WaypointsCount> Waypoints;
	TStaticArray<dtPolyRef, FluxConfig::WaypointsCount> PolyRefs;
	TStaticArray<uint8, FluxConfig::WaypointsCount> Flags;
};

USTRUCT()
struct FFluxPrimeCrowdsCorridor
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
	FFluxPrimeCrowdsCorridor EntityDataCorridors;
	uint8 EntityDataCountCorridor;
	bool EntityDataNeedReplan;
	FFluxPrimeCrowdsWaypoint EntityDataWaypoints;
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