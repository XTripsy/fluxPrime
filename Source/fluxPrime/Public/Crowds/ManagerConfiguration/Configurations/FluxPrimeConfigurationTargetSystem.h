#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseConfiguration.h"
#include "FluxPrimeConfigurationTargetSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeConfigurationTargetSystem : public FFluxPrimeBaseConfiguration
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Show Debug System"))
	bool IsDebug = false;
};