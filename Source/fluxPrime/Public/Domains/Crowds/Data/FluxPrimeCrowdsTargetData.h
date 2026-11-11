#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeCrowdsTargetData.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeTargetCatalog
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AActor> CrowdsTarget;
	
	UPROPERTY()
	FUniqueNetIdRepl CrowdsTargetID;
	
	UPROPERTY(EditAnywhere)
	uint16 CrowdsTargetPriority;
};