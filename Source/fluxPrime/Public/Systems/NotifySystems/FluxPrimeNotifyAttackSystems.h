#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeNotifyAttackSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeNotifyAttackSystems
{
	GENERATED_BODY()
	
	void ExecuteNotify(FFluxPrimeCrowds& members, uint16 indexMember, const FFluxPrimeAnimationNotify& notify)
	{
		if (notify.CrowdTypeNotify != EFluxPrimeCrowdAnimationNotify::NotifyAttack) return;
		if (members.CrowdsID[indexMember] != notify.CrowdIDNotify) return;
		
		UE_LOG(LogTemp, Log, TEXT("NOTIFY:: ATTACK"));
	}
};
