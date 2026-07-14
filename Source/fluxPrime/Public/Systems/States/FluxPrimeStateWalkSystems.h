#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeStateBaseSystems.h"
#include "Cores/FluxPrimeEnum.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeStateWalkSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeStateWalkSystems : public FFluxPrimeStateBaseSystems
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
