#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeCrowdsCompactSystem.generated.h"

USTRUCT(BlueprintType)
struct FCompactSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataIdConfig;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataConditionAliveConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRequestBackToPoolConfig;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRequestSyncAnimationConfig;

	bool IsValid() const
	{
		return EntityDataConditionAliveConfig.EntityDataTag.IsValid() 
		   && EntityDataRequestBackToPoolConfig.EntityDataTag.IsValid()
		   && EntityDataIdConfig.EntityDataTag.IsValid()
		   && EntityDataRequestSyncAnimationConfig.EntityDataTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsCompactSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FCompactSystemDataTagConfig DataTagConfig;
	
	TArray<int16> EntityDataConditionAliveID, EntityDataRequestBackToPoolID, EntityDataIdID, EntityDataRequestSyncAnimationID;
	
private:
	void SwapCrowdsData(FFluxPrimeArchetypeDataStore& dataStore, int32 currentIndex, int32 lastIndex)
	{
		if (currentIndex < 0 || lastIndex < 0) return;
		if (currentIndex == lastIndex) return;
	
		dataStore.SwapManualBuffer(currentIndex, lastIndex);
	}
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		int32 countData = dataStores.Num();
		EntityDataIdID.Init(0, countData);
		EntityDataConditionAliveID.Init(0, countData);
		EntityDataRequestBackToPoolID.Init(0, countData);
		EntityDataRequestSyncAnimationID.Init(0, countData);
		
		int32 indexDataId = 0;
		for (auto& dataStore : dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataIdConfig, EntityDataIdID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataConditionAliveConfig, EntityDataConditionAliveID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRequestBackToPoolConfig, EntityDataRequestBackToPoolID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRequestSyncAnimationConfig, EntityDataRequestSyncAnimationID[indexDataId]);
			indexDataId++;
		}
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Compact_Systems);
		
		if (!IsAllDataValid)
		{
			UE_LOG(LogFluxPrime, Verbose, TEXT("COMPACT SYSTEM :: data invalid; system skipped"));
			return;
		}

		int32 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			FFluxPrimeWrapperBool* entityDataConditionAlive = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataConditionAliveID[indexDataID]));
			FFluxPrimeWrapperBool* entityDataRequestBackToPool = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataRequestBackToPoolID[indexDataID]));
			FFluxPrimeWrapperBool* entityDataRequestSyncAnimation = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataRequestSyncAnimationID[indexDataID]));
			
			check(entityDataConditionAlive);
			check(entityDataRequestBackToPool);
			check(entityDataRequestSyncAnimation);
		
			int32 lastIndex = dataStore.GetActiveEntityCount() - 1;

			for (int32 index = 0; index <= lastIndex;)
			{
				if (!entityDataConditionAlive[index].ValueBool && entityDataRequestBackToPool[index].ValueBool)
				{
					SwapCrowdsData(dataStore, index, lastIndex);
					entityDataRequestSyncAnimation[index].ValueBool = true;
					--lastIndex;
				}
				else ++index;
			}
		
			int16 tempActiveEntityCount = lastIndex + 1;
			dataStore.SetActiveEntityCount(tempActiveEntityCount);
			
			indexDataID++;
		}
	}
};