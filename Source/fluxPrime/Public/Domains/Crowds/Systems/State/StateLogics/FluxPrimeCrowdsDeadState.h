#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/StateMachine/FluxPrimeBaseState.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "FluxPrimeCrowdsDeadState.generated.h"

USTRUCT(BlueprintType)
struct FDeadStateDataTagConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataMaxSpeedConfig;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataMaxSpeedTag;
	
	bool IsValid() const
	{
		return EntityDataMaxSpeedConfig.EntityDataTag.IsValid();
	}
	
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsDeadState : public FFluxPrimeBaseState
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FDeadStateDataTagConfig DataTagConfig;
	
	TArray<int16> EntityDataMaxSpeedID;
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		EntityDataMaxSpeedID.Init(0, dataStores.Num());
		
		int32 indexDataId = 0;
		for (auto& dataStore : dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataMaxSpeedConfig, EntityDataMaxSpeedID[indexDataId]);
			indexDataId++;
		}
	}
	
	void ExecuteState(float deltaTime, int32 indexDataID, int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		if (!IsAllDataValid)
		{
			UE_LOG(LogTemp, Error, TEXT("DEAD STATE :: SOME DATA NOT VALID"));
			return;
		}
		
		FFluxPrimeWrapperFloat* entityDataMaxSpeed = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataMaxSpeedID[indexDataID]));
		
		entityDataMaxSpeed[indexEntity].ValueFloat = 0;
	}
};
