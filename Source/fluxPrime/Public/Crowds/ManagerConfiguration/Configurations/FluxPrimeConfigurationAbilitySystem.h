#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseConfiguration.h"
#include "StructUtils/InstancedStruct.h"
#include "FluxPrimeConfigurationAbilitySystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeConfigurationAbilitySystem : public FFluxPrimeBaseConfiguration
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Show Debug System"))
	bool IsDebug = false;
	
	UPROPERTY(EditAnywhere, meta=(BaseStruct="FluxPrimeBaseAbilitySystems", ExcludeBaseStruct))
	TArray<FInstancedStruct> ConfigurationAbilityFragments;
};