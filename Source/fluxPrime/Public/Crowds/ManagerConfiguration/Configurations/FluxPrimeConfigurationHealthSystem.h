#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseConfiguration.h"
#include "FluxPrimeConfigurationHealthSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeConfigurationHealthSystem : public FFluxPrimeBaseConfiguration
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Show Debug System"))
	bool IsDebug = false;
};