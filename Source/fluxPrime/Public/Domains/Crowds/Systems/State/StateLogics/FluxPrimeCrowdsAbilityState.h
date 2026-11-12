#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Framework/Systems/StateMachine/FluxPrimeBaseState.h"
#include "FluxPrimeCrowdsAbilityState.generated.h"

USTRUCT(BlueprintType)
struct FAbilityStateDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataCurrentWaypointLocationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLocationConfig;

	bool IsValid() const
	{
		return EntityDataCurrentWaypointLocationConfig.EntityDataTag.IsValid() 
		   && EntityDataLocationConfig.EntityDataTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsAbilityState : public FFluxPrimeBaseState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FAbilityStateDataTagConfig DataTagConfig;

	TArray<int16> EntityDataCurrentWaypointLocationID, EntityDataLocationID;
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		int32 countData = dataStores.Num();
		EntityDataCurrentWaypointLocationID.Init(0, countData);
		EntityDataLocationID.Init(0, countData);
		
		int32 indexDataId = 0;
		for (auto& dataStore : dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataCurrentWaypointLocationConfig, EntityDataCurrentWaypointLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataLocationConfig, EntityDataLocationID[indexDataId]);
			indexDataId++;
		}
	}
	
	void ExecuteState(float deltaTime, int32 indexDataID, int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		if (!IsAllDataValid)
		{
			UE_LOG(LogFluxPrime, Verbose, TEXT("ABILITY STATE :: data invalid; system skipped"));
			return;
		}
		
		FVector* entityDataCurrentWaypointLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointLocationID[indexDataID]));
		FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
		
		entityDataCurrentWaypointLocation[indexEntity] = entityDataLocation[indexEntity]; 
	}
};
