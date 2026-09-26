#pragma once

#include "CoreMinimal.h"
#include "Detour/DetourNavMesh.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsConfigData.h"
#include "FluxPrimeNavigationData.generated.h"

// ini perlu di ubah
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
	uint8 PriorityScore;
	int16 DataStoreIndex;
	int32 InternalIndex;
	FFluxPrimeNavigationTask DataTask;
    
	bool operator<(const FFluxPrimeNavigationDataRequest& Other) const
	{
		return PriorityScore > Other.PriorityScore;
	}
};