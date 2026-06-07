#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseStateSystems.h"
#include "Cores/FluxPrimeEnum.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeWalkSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeWalkSystems : public FFluxPrimeBaseStateSystems
{
	GENERATED_BODY()
	
	void UpdateWalkSystems(FFluxPrimeCrowds& members, const int32 memberActive)
	{
		for (int i = 0; i < memberActive; ++i)
		{
			if (members.CrowdsState[i] != EFluxPrimeCrowdState::StateWalk) continue;
			
			UpdateAnimation(members, i);
			
			UE_LOG(LogTemp, Log, TEXT("WalkSystems: WALK:: index %d"), i);
		}
	}
};
