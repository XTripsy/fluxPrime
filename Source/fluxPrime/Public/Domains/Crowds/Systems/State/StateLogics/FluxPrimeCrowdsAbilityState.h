#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/StateMachine/FluxPrimeBaseState.h"
#include "FluxPrimeCrowdsAbilityState.generated.h"

USTRUCT(BlueprintType)
struct FAbilityStateDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataCurrentWaypointLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLocationTag;

	bool IsValid() const
	{
		return EntityDataCurrentWaypointLocationTag.IsValid() 
		   && EntityDataLocationTag.IsValid();
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
		for (auto& dataStore : dataStores)
		{
			EntityDataCurrentWaypointLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCurrentWaypointLocationTag));
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
		}
	}
	
	void ExecuteState(float deltaTime, int32 indexDataID, int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		FVector* entityDataCurrentWaypointLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointLocationID[indexDataID]));
		FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
		
		entityDataCurrentWaypointLocation[indexEntity] = entityDataLocation[indexEntity]; 
		UE_LOG(LogTemp, Log, TEXT("STATE:: ABILITY LOGIC STATE"));
	}
};
