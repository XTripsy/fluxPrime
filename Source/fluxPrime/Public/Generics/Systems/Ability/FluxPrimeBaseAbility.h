#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FluxPrimeBaseAbility.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeBaseAbility
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FGameplayTag Identity;
};