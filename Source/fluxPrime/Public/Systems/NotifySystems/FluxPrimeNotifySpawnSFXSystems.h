#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeNotifySpawnSFXSystems.generated.h"

USTRUCT()
struct FFluxPrimeNotifySpawnSFXSystemsContext
{
	GENERATED_BODY()
	
	TArray<EFluxPrimeCrowdAnimationNotify>* requestNotifyCrowds = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeNotifySpawnSFXSystems : public FFluxPrimeNotifyBaseSystems
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
		
		UE_LOG(LogTemp, Log, TEXT("NOTIFY:: SPAWN SFX"));
	}
};
