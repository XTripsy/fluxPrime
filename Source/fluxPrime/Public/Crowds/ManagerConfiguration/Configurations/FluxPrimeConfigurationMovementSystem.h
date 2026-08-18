#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseConfiguration.h"
#include "FluxPrimeConfigurationMovementSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeConfigurationMovementSystem  : public FFluxPrimeBaseConfiguration
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Show Debug System"))
	bool IsDebug = false;
};