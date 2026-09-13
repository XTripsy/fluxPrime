#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/StateMachine/FluxPrimeBaseState.h"
#include "FluxPrimeCrowdsWalkState.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsWalkState : public FFluxPrimeBaseState
{
	GENERATED_BODY()
	
	void ExecuteState(float deltaTime, int32 indexDataID, int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		UE_LOG(LogTemp, Log, TEXT("STATE:: WALK LOGIC STATE"));
	}
};
