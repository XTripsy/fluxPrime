#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeStateBaseSystem.h"
#include "Cores/FluxPrimeEnum.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeStateWalkSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeStateWalkSystem : public FFluxPrimeStateBaseSystem
{
	GENERATED_BODY()
	
	void UpdateWalkSystems(FFluxPrimeCrowds& members, const int32 memberActive)
	{
		for (int i = 0; i < memberActive; ++i)
		{
			if (members.CrowdsState[i] != EFluxPrimeCrowdState::StateWalk) continue;
			
			UE_LOG(LogTemp, Log, TEXT("WalkSystems: WALK:: index %d"), i);
		}
	}
};
