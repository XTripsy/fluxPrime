#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeNotifySpawnVFXSystems.generated.h"

USTRUCT()
struct FFluxPrimeNotifySpawnVFXSystemsContext
{
	GENERATED_BODY()
	
	TArray<EFluxPrimeCrowdAnimationNotify>* requestNotifyCrowds = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeNotifySpawnVFXSystems : public FFluxPrimeNotifyBaseSystems
{
	GENERATED_BODY()
	
	void InitializeNotifySpawnVFXSystems(FFluxPrimeNotifySpawnVFXSystemsContext context)
	{
		check(context.requestNotifyCrowds);
		
		RequestNotifyCrowds = context.requestNotifyCrowds;
	}
	
	void ExecuteNotify(const uint16 indexMember)
	{
		if ((*RequestNotifyCrowds)[indexMember] != EFluxPrimeCrowdAnimationNotify::NotifySpawnVFX) return;
		
		UE_LOG(LogTemp, Log, TEXT("NOTIFY:: SPAWN VFX"));
	}
};
