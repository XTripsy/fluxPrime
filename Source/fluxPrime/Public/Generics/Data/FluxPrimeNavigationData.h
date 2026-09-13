#pragma once

#include "CoreMinimal.h"
#include "Detour/DetourNavMesh.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsConfigData.h"
#include "FluxPrimeNavigationData.generated.h"

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
