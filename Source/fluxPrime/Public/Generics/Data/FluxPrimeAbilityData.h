#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeAbilityData.generated.h"

USTRUCT()
struct FAbilityFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta=(BaseStruct="FluxPrimeBaseAbilitySystems", ExcludeBaseStruct))
	TObjectPtr<UScriptStruct> AbilityFragment;
	
	UPROPERTY(EditAnywhere)
	FGuid ID;
};