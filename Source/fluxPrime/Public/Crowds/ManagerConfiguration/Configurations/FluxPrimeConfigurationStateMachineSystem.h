#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseConfiguration.h"
#include "FluxPrimeConfigurationStateMachineSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeConfigurationStateMachineSystem : public FFluxPrimeBaseConfiguration
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Show Debug System"))
	bool IsDebug = false;
};