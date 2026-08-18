#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseAbilitySystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeBaseAbilitySystem
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	FGuid ID;
	
	UPROPERTY()
	bool IsActive = false;
	
	FFluxPrimeBaseAbilitySystem()
	{
		ID = FGuid::NewGuid();
	}
};