#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeStateBaseSystem.h"
#include "Cores/FluxPrimeEnum.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeStateAbilitySystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeStateAbilitySystem : public FFluxPrimeStateBaseSystem
{
	GENERATED_BODY()
	
	void UpdateAbilitySystems(FFluxPrimeCrowds& members, const int32 memberActive)
	{
		for (int i = 0; i < memberActive; ++i)
		{
			if (members.CrowdsState[i] != EFluxPrimeCrowdState::StateAbility) continue;
			
			UE_LOG(LogTemp, Log, TEXT("AbilitySystems: ATTACK:: index %d"), i);
		}
	}
};
