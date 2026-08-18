#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool IsActive = false;
};
