#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseAbility.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeBaseAbility
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	FGuid ID;
	
	FFluxPrimeBaseAbility()
	{
		ID = FGuid::NewGuid();
	}
};