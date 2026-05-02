#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseConfiguration.h"
#include "FluxPrimeConfigurationBoidsSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeConfigurationBoidsSystems : public FFluxPrimeBaseConfiguration
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "SeparationWeight"))
	float SeparationWeight = 0.0f;
};