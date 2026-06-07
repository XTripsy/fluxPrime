#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeNotifySpawnVFXSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeNotifySpawnVFXSystems
{
	GENERATED_BODY()
	
	void ExecuteNotify(FFluxPrimeCrowds& members, uint16 indexMember, const FFluxPrimeAnimationNotify& notify)
	{
		if (notify.CrowdTypeNotify != EFluxPrimeCrowdAnimationNotify::NotifySpawnVFX) return;
		if (members.CrowdsID[indexMember] != notify.CrowdIDNotify) return;
		
		UE_LOG(LogTemp, Log, TEXT("NOTIFY:: SPAWN VFX"));
	}
};
