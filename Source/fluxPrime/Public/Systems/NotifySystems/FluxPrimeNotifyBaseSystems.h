#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeNotifyBaseSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeNotifyBaseSystems
{
	GENERATED_BODY()
	
protected:
	TArray<EFluxPrimeCrowdAnimationNotify>* RequestNotifyCrowds = nullptr;
};
