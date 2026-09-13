#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "FluxPrimeHealthSystem.generated.h"

USTRUCT(BlueprintType)
struct FHealthSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataConditionAliveTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataHealthTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataDamageTakenTag;

	bool IsValid() const
	{
		return EntityDataConditionAliveTag.IsValid() 
		   && EntityDataHealthTag.IsValid() 
		   && EntityDataDamageTakenTag.IsValid();
	}
};

USTRUCT()
struct FFluxPrimeHealthSystemContext
{
	GENERATED_BODY()
	
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeHealthSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FHealthSystemDataTagConfig DataTagConfig;

	TArray<int16> EntityDataConditionAliveID, EntityDataHealthID, EntityDataDamageTakenID;
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		for (auto& dataStore : dataStores)
		{
			EntityDataConditionAliveID.Add(dataStore.GetDataID(DataTagConfig.EntityDataConditionAliveTag));
			EntityDataHealthID.Add(dataStore.GetDataID(DataTagConfig.EntityDataHealthTag));
			EntityDataDamageTakenID.Add(dataStore.GetDataID(DataTagConfig.EntityDataDamageTakenTag));
		}
	}
	
	void TakeDamage(FFluxPrimeArchetypeDataStore& dataStore, int32 indexDataID, int32 indexData, int16 damageTaken)
	{
		reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataDamageTakenID[indexDataID]))[indexData].ValueInt16 = damageTaken;
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Health_Systems);
		
		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			FFluxPrimeWrapperBool* entityDataConditionAlive = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataConditionAliveID[indexDataID]));
			FFluxPrimeWrapperInt16* entityDataHealth = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataHealthID[indexDataID]));
			FFluxPrimeWrapperInt16* entityDataDamageTaken = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataDamageTakenID[indexDataID]));
		
			for (int i = 0; i < dataStore.GetActiveEntityCount(); ++i)
			{
				entityDataHealth[i].ValueInt16 -= entityDataDamageTaken[i].ValueInt16;
				entityDataConditionAlive[i].ValueBool = entityDataHealth[i].ValueInt16 > 0;
				entityDataDamageTaken[i].ValueInt16 = 0;
			}
			
			indexDataID++;
		}
	}
};