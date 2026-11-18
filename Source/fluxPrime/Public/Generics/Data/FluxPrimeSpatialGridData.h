#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeSpatialGridData.generated.h"

USTRUCT()
struct FFluxPrimeGridMemberData
{
	GENERATED_BODY()
	
	int16 EntityType;
	int32 EntityIndex;
	/** Copied from the location buffer when the grid is rebuilt, so consumers that only
	 *  need the position (the boids separation pass) avoid a second dependent load. */
	float LocationX;
	float LocationY;
};
