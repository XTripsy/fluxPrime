#pragma once

#include "CoreMinimal.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "FluxPrimeBaseState.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeBaseState
{
	GENERATED_BODY()
	
	/* turunan struct ini perlu implement function dengan nama yang sama, untuk dapat mengeksekusi logic statenya
	void ExecuteState(float deltaTime)
	{
		
	}*/
};
