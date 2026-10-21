#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeCrowdsTargetData.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsTargetData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FUniqueNetIdRepl PlayerID;
	
	UPROPERTY(EditAnywhere)
	int16 PlayerPriority;
};