#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeNotifyBase.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeNotifyBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	FGameplayTag NotifyTag;
	
	UPROPERTY()
	int32 NotifyID = INDEX_NONE;
};
