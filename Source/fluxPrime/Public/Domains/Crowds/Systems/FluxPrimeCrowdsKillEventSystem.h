#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "FluxPrimeCrowdsKillEventSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsKillEventSystemDataTagConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataHealthConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataDamageTakenConfig;

	bool IsValid() const
	{
		return EntityDataHealthConfig.EntityDataTag.IsValid() 
		   && EntityDataDamageTakenConfig.EntityDataTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsKillEventSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FFluxPrimeCrowdsKillEventSystemDataTagConfig DataTagConfig;

	TArray<int16> EntityDataHealthID, EntityDataDamageTakenID;
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		int32 countData = dataStores.Num();
		EntityDataHealthID.Init(0, countData);
		EntityDataDamageTakenID.Init(0, countData);
		
		int32 indexDataId = 0;
		for (auto& dataStore : dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataHealthConfig, EntityDataHealthID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataDamageTakenConfig, EntityDataDamageTakenID[indexDataId]);
			indexDataId++;
		}
	}
	
	bool CekKillEvent(FFluxPrimeArchetypeDataStore& dataStore, int32 indexDataID, int32 indexData, int16 damageTaken)
	{
		if (!IsAllDataValid)
		{
			UE_LOG(LogTemp, Error, TEXT("KILL EVENT SYSTEM :: SOME DATA NOT VALID"));
			return false;
		}
		
		int16 tempValue = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataHealthID[indexDataID]))[indexData].ValueInt16 - reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataDamageTakenID[indexDataID]))[indexData].ValueInt16;
		return tempValue <= 0;
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		
	}
};