#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseConfiguration.h"
#include "FluxPrimeConfigurationSpatialGridSystem.h"
#include "FluxPrimeConfigurationBoidsSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeConfigurationBoidsSystem : public FFluxPrimeBaseConfiguration
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "SeparationWeight"))
	float SeparationWeight = 0.0f;
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "SpatialGrid"))
	FFluxPrimeConfigurationSpatialGridSystem ConfigurationSpatialGrid;
};