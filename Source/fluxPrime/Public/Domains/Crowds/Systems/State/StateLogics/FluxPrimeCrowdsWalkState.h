#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Framework/Systems/StateMachine/FluxPrimeBaseState.h"
#include "FluxPrimeCrowdsWalkState.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsWalkState : public FFluxPrimeBaseState
{
	GENERATED_BODY()
	
	void ExecuteState(float deltaTime, int32 indexDataID, int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		if (!IsAllDataValid)
		{
			UE_LOG(LogFluxPrime, Verbose, TEXT("WALK STATE :: data invalid; system skipped"));
			return;
		}
	}
};
