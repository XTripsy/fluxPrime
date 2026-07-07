#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeNotifyDeadSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeNotifyDeadSystems
{
	GENERATED_BODY()
	
	void ExecuteNotify(FFluxPrimeCrowds& members, uint16 indexMember, const FFluxPrimeAnimationNotify& notify)
	{
		if (notify.CrowdTypeNotify != EFluxPrimeCrowdAnimationNotify::NotifyDead) return;
		if (members.CrowdsID[indexMember] != notify.CrowdIDNotify) return;
		
		UE_LOG(LogTemp, Log, TEXT("NOTIFY:: DEAD| Index %d"), indexMember);
		members.CrowdsRequestBackToPool[indexMember] = true;
		members.CrowdsLocation[indexMember] = FVector::DownVector * 1000.0f;
	}
};
