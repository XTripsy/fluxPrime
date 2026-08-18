#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseConfiguration.h"
#include "FluxPrimeConfigurationSpatialGridSystem.h"
#include "FluxPrimeConfigurationAnimationSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeConfigurationAnimationSystem : public FFluxPrimeBaseConfiguration
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Show Debug System"))
	bool IsDebug = false;
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "SpatialGrid"))
	FFluxPrimeConfigurationSpatialGridSystem ConfigurationSpatialGrid;
};