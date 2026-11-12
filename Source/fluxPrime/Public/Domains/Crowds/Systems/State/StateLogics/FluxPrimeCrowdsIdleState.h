#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Framework/Systems/StateMachine/FluxPrimeBaseState.h"
#include "FluxPrimeCrowdsIdleState.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsIdleState : public FFluxPrimeBaseState
{
	GENERATED_BODY()
	
	void ExecuteState(float deltaTime, int32 indexDataID, int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		if (!IsAllDataValid)
		{
			UE_LOG(LogFluxPrime, Verbose, TEXT("IDLE STATE :: data invalid; system skipped"));
			return;
		}
	}
};
