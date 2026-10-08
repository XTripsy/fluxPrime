#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeCrowdsCompactSystem.generated.h"

USTRUCT(BlueprintType)
struct FCompactSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataConditionAliveTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRequestBackToPoolTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataIdTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRequestSyncAnimationTag;

	bool IsValid() const
	{
		return EntityDataConditionAliveTag.IsValid() 
		   && EntityDataRequestBackToPoolTag.IsValid()
		   && EntityDataIdTag.IsValid()
		   && EntityDataRequestSyncAnimationTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsCompactSystemsContext
{
	GENERATED_BODY()
	
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
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
	void UpdateLookUpData(FFluxPrimeArchetypeDataStore& dataStore, int32 indexLookUpData, int32 newIndexLookUpDaata)
	{
		dataStore.UpdateLookUpValue(indexLookUpData, newIndexLookUpDaata);
	}
	
	void SwapCrowdsData(FFluxPrimeArchetypeDataStore& dataStore, int32 currentIndex, int32 lastIndex)
	{
		if (currentIndex < 0 || lastIndex < 0) return;
		if (currentIndex == lastIndex) return;
	
		dataStore.SwapManualBuffer(currentIndex, lastIndex);
	}
	
public:
	void Initialized(FFluxPrimeCrowdsCompactSystemsContext context)
	{
		check(context.dataStores);
	
		for (auto& dataStore : *context.dataStores)
		{
			EntityDataConditionAliveID.Add(dataStore.GetDataID(DataTagConfig.EntityDataConditionAliveTag));
			EntityDataRequestBackToPoolID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestBackToPoolTag));
			EntityDataIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataIdTag));
			EntityDataRequestSyncAnimationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestSyncAnimationTag));
		}
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Compact_Systems);

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
					UpdateLookUpData(dataStore, index, lastIndex);
					UpdateLookUpData(dataStore, lastIndex, index);
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