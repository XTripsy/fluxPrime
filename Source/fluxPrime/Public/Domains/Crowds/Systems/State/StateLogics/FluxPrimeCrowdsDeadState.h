#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/StateMachine/FluxPrimeBaseState.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "FluxPrimeCrowdsDeadState.generated.h"

USTRUCT(BlueprintType)
struct FDeadStateDataTagConfig
{
	GENERATED_BODY()
	
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsDeadState : public FFluxPrimeBaseState
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FDeadStateDataTagConfig DataTagConfig;
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{

	}
	
	void ExecuteState(float deltaTime, int32 indexDataID, int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		UE_LOG(LogTemp, Log, TEXT("STATE:: DEAD LOGIC STATE"));
	}
};
