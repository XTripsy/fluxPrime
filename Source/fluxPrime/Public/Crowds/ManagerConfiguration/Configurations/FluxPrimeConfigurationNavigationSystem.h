#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseConfiguration.h"
#include "FluxPrimeConfigurationNavigationSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeConfigurationNavigationSystem : public FFluxPrimeBaseConfiguration
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Show Debug System"))
	bool IsDebug = false;
	
	UPROPERTY(EditAnywhere)
	uint16 QueuePathCountPerFrame = 20;
	UPROPERTY(EditAnywhere)
	uint16 QueueCorridorCountPerFrame = 20;
	
	UPROPERTY(EditAnywhere)
	float OptimizeTimeSameCellID = 0.15f;
	UPROPERTY(EditAnywhere)
	float OptimizeTimeDifferenceCellID = 0.5f;
	
	UPROPERTY(EditAnywhere)
	float MoveTargetTimeSameCellID = 0.25f;
	UPROPERTY(EditAnywhere)
	float MoveTargetTimeDifferenceCellID = 1.5f;
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "SpatialGrid"))
	FFluxPrimeConfigurationSpatialGridSystem ConfigurationSpatialGrid;
};