#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeNotifyBaseSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeNotifyBaseSystem
{
	GENERATED_BODY()
	
protected:
	TArray<EFluxPrimeCrowdAnimationNotify>* RequestNotifyCrowds = nullptr;
};
