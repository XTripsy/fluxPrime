#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystems.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeHealthSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeHealthSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()
	
	void UpdateHealthSystems(FFluxPrimeCrowds& members, int32 memberActive)
	{
		for (int i = 0; i < memberActive; ++i)
		{
			if (members.CrowdsHealth[i] > 0) continue;
			
			// do anim
		}
	}
};