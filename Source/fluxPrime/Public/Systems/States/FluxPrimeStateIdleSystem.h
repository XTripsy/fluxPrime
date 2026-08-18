#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeStateBaseSystem.h"
#include "Cores/FluxPrimeEnum.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeStateIdleSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeStateIdleSystem : public FFluxPrimeStateBaseSystem
{
	GENERATED_BODY()
	
	void UpdateIdleSystems(FFluxPrimeCrowds& members, const int32 memberActive)
	{
		for (int i = 0; i < memberActive; ++i)
		{
			if (members.CrowdsState[i] != EFluxPrimeCrowdState::StateIdle) continue;
			
			UE_LOG(LogTemp, Log, TEXT("IdleSystems: IDLE:: index %d"), i);
		}
	}
};
