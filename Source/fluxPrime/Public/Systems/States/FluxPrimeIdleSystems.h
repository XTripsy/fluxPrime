#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseStateSystems.h"
#include "Cores/FluxPrimeEnum.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeIdleSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeIdleSystems : public FFluxPrimeBaseStateSystems
{
	GENERATED_BODY()
	
	void UpdateIdleSystems(FFluxPrimeCrowds& members, const int32 memberActive)
	{
		for (int i = 0; i < memberActive; ++i)
		{
			if (members.CrowdsState[i] != EFluxPrimeCrowdState::StateIdle) continue;
			
			UpdateAnimation(members, i);
			
			UE_LOG(LogTemp, Log, TEXT("IdleSystems: IDLE:: index %d"), i);
		}
	}
};
