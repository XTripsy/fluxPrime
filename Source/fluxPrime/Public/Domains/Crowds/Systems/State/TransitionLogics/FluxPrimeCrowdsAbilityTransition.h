#pragma once

#include "CoreMinimal.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Systems/StateMachine/FluxPrimeBaseTransition.h"
#include "FluxPrimeCrowdsAbilityTransition.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsAbilityTransition : public FFluxPrimeBaseTransition
{
	GENERATED_BODY()
	
	bool ExecuteTransition(int32 indexDataID, int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		UE_LOG(LogTemp, Log, TEXT("TRANSITION:: LOGIC TRANSITION"));
		return true;
	}
};
