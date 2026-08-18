#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeNotifySpawnSFXSystem.generated.h"

USTRUCT()
struct FFluxPrimeNotifySpawnSFXSystemsContext
{
	GENERATED_BODY()
	
	TArray<EFluxPrimeCrowdAnimationNotify>* requestNotifyCrowds = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeNotifySpawnSFXSystem : public FFluxPrimeNotifyBaseSystem
{
	GENERATED_BODY()
	
	void InitializeNotifySpawnSFXSsystems(FFluxPrimeNotifySpawnSFXSystemsContext context)
	{
		check(context.requestNotifyCrowds);
		
		RequestNotifyCrowds = context.requestNotifyCrowds;
	}
	
	void ExecuteNotify(const uint16 indexMember)
	{
		if ((*RequestNotifyCrowds)[indexMember] != EFluxPrimeCrowdAnimationNotify::NotifySpawnSFX) return;
		
		(*RequestNotifyCrowds)[indexMember] = EFluxPrimeCrowdAnimationNotify::NotifyNone;
		UE_LOG(LogTemp, Log, TEXT("NOTIFY:: SPAWN SFX"));
	}
};
