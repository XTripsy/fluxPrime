#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperBool.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeHealthSystem.generated.h"

USTRUCT(BlueprintType)
struct FHealthSystemDataTagConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataConditionAliveConfig;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataHealthConfig;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataDamageTakenConfig;

	bool IsValid() const
	{
		return EntityDataConditionAliveConfig.EntityDataTag.IsValid() 
		   && EntityDataHealthConfig.EntityDataTag.IsValid()
		   && EntityDataDamageTakenConfig.EntityDataTag.IsValid();
	}
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
		int32 countData = dataStores.Num();
		EntityDataConditionAliveID.Init(0, countData);
		EntityDataHealthID.Init(0, countData);
		EntityDataDamageTakenID.Init(0, countData);
		
		int32 indexDataId = 0;
		for (auto& dataStore : dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataConditionAliveConfig, EntityDataConditionAliveID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataHealthConfig, EntityDataHealthID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataDamageTakenConfig, EntityDataDamageTakenID[indexDataId]);
			indexDataId++;
		}
	}
	
	void TakeDamage(FFluxPrimeArchetypeDataStore& dataStore, int32 indexDataID, int32 indexData, int16 damageTaken)
	{
		if (!IsAllDataValid)
		{
			UE_LOG(LogTemp, Error, TEXT("HEALTH SYSTEM :: SOME DATA NOT VALID"));
			return;
		}
		
		reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataDamageTakenID[indexDataID]))[indexData].ValueInt16 += damageTaken;
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Health_Systems);
		
		if (!IsAllDataValid)
		{
			UE_LOG(LogTemp, Error, TEXT("HEALTH SYSTEM :: SOME DATA NOT VALID"));
			return;
		}
		
		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			FFluxPrimeWrapperBool* entityDataConditionAlive = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataConditionAliveID[indexDataID]));
			FFluxPrimeWrapperInt16* entityDataHealth = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataHealthID[indexDataID]));
			FFluxPrimeWrapperInt16* entityDataDamageTaken = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataDamageTakenID[indexDataID]));
		
			check(entityDataConditionAlive);
			check(entityDataHealth);
			check(entityDataDamageTaken);
			
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