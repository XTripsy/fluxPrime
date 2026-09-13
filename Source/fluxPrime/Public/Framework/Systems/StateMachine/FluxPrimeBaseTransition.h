#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FluxPrimeBaseTransition.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeBaseTransition
{
	GENERATED_BODY()
	
	/* turunan struct ini perlu implement function dengan nama yang sama, untuk dapat mengeksekusi logic transitionnya
	bool ExecuteTransition()
	{
		return false;
	}*/
};
