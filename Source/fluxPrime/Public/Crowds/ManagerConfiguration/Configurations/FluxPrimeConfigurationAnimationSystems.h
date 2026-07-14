#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseConfiguration.h"
#include "FluxPrimeConfigurationSpatialGridSystems.h"
#include "FluxPrimeConfigurationAnimationSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeConfigurationAnimationSystems : public FFluxPrimeBaseConfiguration
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Show Debug System"))
	bool IsDebug = false;
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "SpatialGrid"))
	FFluxPrimeConfigurationSpatialGridSystems ConfigurationSpatialGrid;
};