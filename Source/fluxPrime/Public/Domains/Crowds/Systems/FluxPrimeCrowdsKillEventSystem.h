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
	FGameplayTag EntityDataHealthTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataDamageTakenTag;

	bool IsValid() const
	{
		return EntityDataHealthTag.IsValid() 
		   && EntityDataDamageTakenTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsKillEventSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FFluxPrimeCrowdsKillEventSystemDataTagConfig DataTagConfig;

	TArray<int16> EntityDataConditionAliveID, EntityDataHealthID, EntityDataDamageTakenID;
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		for (auto& dataStore : dataStores)
		{
			EntityDataHealthID.Add(dataStore.GetDataID(DataTagConfig.EntityDataHealthTag));
			EntityDataDamageTakenID.Add(dataStore.GetDataID(DataTagConfig.EntityDataDamageTakenTag));
		}
	}
	
	bool CekKillEvent(FFluxPrimeArchetypeDataStore& dataStore, int32 indexDataID, int32 indexData, int16 damageTaken)
	{
		int16 tempValue = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataHealthID[indexDataID]))[indexData].ValueInt16 - reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataDamageTakenID[indexDataID]))[indexData].ValueInt16;
		return tempValue <= 0;
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		
	}
};