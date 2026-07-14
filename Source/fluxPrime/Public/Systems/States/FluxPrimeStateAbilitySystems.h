#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeStateBaseSystems.h"
#include "Cores/FluxPrimeEnum.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeStateAbilitySystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeStateAbilitySystems : public FFluxPrimeStateBaseSystems
{
	GENERATED_BODY()
	
	void UpdateAbilitySystems(FFluxPrimeCrowds& members, const int32 memberActive)
	{
		for (int i = 0; i < memberActive; ++i)
		{
			if (members.CrowdsState[i] != EFluxPrimeCrowdState::StateAbility) continue;
			
			UpdateAnimation(members, i);
			
			UE_LOG(LogTemp, Log, TEXT("AbilitySystems: ATTACK:: index %d"), i);
		}
	}
};
