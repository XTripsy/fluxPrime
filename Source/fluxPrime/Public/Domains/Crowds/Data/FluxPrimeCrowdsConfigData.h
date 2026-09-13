#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeCrowdsConfigData.generated.h"

namespace FluxConfig
{
	constexpr int8 AnimationArrayCount = 8;
	constexpr int8 NavigationArrayCount = 8;
	constexpr int8 WaypointsCount = 16;
	constexpr int8 CorridorsCount = 64;
	constexpr float DebugLocationMovement = 620.0f;
	constexpr float DebugLocationIdentity = 600.0f;
	constexpr float DebugLocationAnimation = 520.0f;
	constexpr float DebugLocationNavigation = 440.0f;
	constexpr float DebugLocationSpatialGrid = 360.0f;
	constexpr float DebugScaleFont = .8f;
}

USTRUCT()
struct FFluxPrimeCrowdsConfigData
{
	GENERATED_BODY()
	
};
