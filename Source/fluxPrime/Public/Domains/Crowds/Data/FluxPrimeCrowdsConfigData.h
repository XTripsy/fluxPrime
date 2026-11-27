#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeCrowdsConfigData.generated.h"

namespace FluxConfig
{
	// Legacy compile-time capacity; changing fixed buffer capacities requires a layout audit.
	constexpr int8 AnimationArrayCount = 8;
	constexpr int8 NavigationArrayCount = 8;
	constexpr int8 WaypointsCount = 16;
	constexpr int8 CorridorsCount = 64;
}

USTRUCT()
struct FFluxPrimeCrowdsConfigData
{
	GENERATED_BODY()
	
};
