#pragma once

#include "CoreMinimal.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "FluxPrimeBaseSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
	virtual ~FFluxPrimeBaseSystem() = default;
};
