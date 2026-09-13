#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "Domains/Crowds/Event/FluxPrimeCrowdsEvent.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsData.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "GameplayTagContainer.h"
#if IF_WITH_EDITOR
#include "ProfilingDebugging/CpuProfilerTrace.h"
#endif
#include "FluxPrimeCrowdsTargetSystem.generated.h"

USTRUCT(BlueprintType)
struct FTargetSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataTargetLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataTargetIdTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataIdTag;

	bool IsValid() const
	{
		return EntityDataLocationTag.IsValid() 
			&& EntityDataTargetLocationTag.IsValid() 
			&& EntityDataTargetIdTag.IsValid() 
			&& EntityDataIdTag.IsValid();
	}
};

USTRUCT()
struct FFluxPrimeCrowdsTargetSystemsContext
{
	GENERATED_BODY()
	
	TArray<FFluxPrimeTargetCatalog>* targetCatalogs = nullptr;
	TArray<float>* aggroDistance = nullptr;
	FOnCrowdsManagerActionChange* onManagerActionChange = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsTargetSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	TArray<FFluxPrimeTargetCatalog>* TargetCatalogs = nullptr;
	FOnCrowdsManagerActionChange* OnManagerActionChange = nullptr;
	
	UPROPERTY()
	TArray<float> AggroDistance;
	
public:
	UPROPERTY(EditAnywhere)
	FTargetSystemDataTagConfig DataTagConfig;
	
	TArray<int16> EntityDataLocationID, EntityDataTargetLocationID, EntityDataTargetIdID, EntityDataIdID;
	
public:
	void Initialized(FFluxPrimeCrowdsTargetSystemsContext context)
	{
		check(context.targetCatalogs);
		check(context.onManagerActionChange);
		check(context.dataStores);
		
		TargetCatalogs = context.targetCatalogs;
		AggroDistance = *context.aggroDistance;
		OnManagerActionChange = context.onManagerActionChange;
		
		for (auto& dataStore : *context.dataStores)
		{
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataTargetLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataTargetLocationTag));
			EntityDataTargetIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataTargetIdTag));
			EntityDataIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataIdTag));
		}
	}
	
	void SyncNewTarget(FFluxPrimeArchetypeDataStore& dataStore, int32 indexDataID, int32 indexData, int16 newTargetID, FVector newTargetLocation)
	{
		reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]))[indexData] = newTargetLocation;
		reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataTargetIdID[indexDataID]))[indexData].ValueInt16 = newTargetID;
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
#if IF_WITH_EDITOR
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Target_Systems);
#endif

		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
			FVector* entityDataTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]));
			FFluxPrimeWrapperInt16* entityDataTargetID = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataTargetIdID[indexDataID]));
			FFluxPrimeWrapperInt16* entityDataID = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataIdID[indexDataID]));
			auto& targetCatalogs = *TargetCatalogs;
			
			TArray<FVector> targetLocations;
			TArray<uint16> targetPriority;
			targetLocations.SetNumUninitialized(targetCatalogs.Num());
			targetPriority.SetNumUninitialized(targetCatalogs.Num());

			for (int32 i = 0; i < targetCatalogs.Num(); ++i)
			{
				if (!targetCatalogs[i].CrowdsTarget.IsValid()) continue;

				targetLocations[i] = targetCatalogs[i].CrowdsTarget->GetActorLocation();
				targetPriority[i] = targetCatalogs[i].CrowdsTargetPriority;
			}
			
			float bestDistanceSquared = TNumericLimits<float>::Max();
			int32 bestPriority = TNumericLimits<int32>::Lowest();
			int16 bestTargetID = INDEX_NONE;
			
			for (int32 i = 0; i < dataStore.GetActiveEntityCount(); ++i)
			{
				bestDistanceSquared = TNumericLimits<float>::Max();
				bestPriority = TNumericLimits<int32>::Lowest();
				bestTargetID = INDEX_NONE;

				for (int j = 0; j < targetCatalogs.Num(); ++j)
				{
					const float distanceSquared = FVector::DistSquaredXY(entityDataLocation[i], targetLocations[j]);
					const float aggroDistanceSquared = FMath::Square(AggroDistance[indexDataID]);
					
					if (distanceSquared > aggroDistanceSquared) continue;
					
					const int32 priority = targetPriority[j];
					
					if (priority > bestPriority)
					{
						bestPriority = priority;
						bestDistanceSquared = distanceSquared;
						bestTargetID = j;

						continue;
					}

					if (priority == bestPriority &&
						distanceSquared < bestDistanceSquared)
					{
						bestDistanceSquared = distanceSquared;
						bestTargetID = j;
					}
				}

				if (entityDataTargetID[i].ValueInt16 != bestTargetID && bestTargetID != INDEX_NONE)
				{
					entityDataTargetID[i].ValueInt16 = static_cast<uint16>(bestTargetID);
					
					FFluxPrimeChangeTargetActionPayload data;
					data.CrowdID = entityDataID[i].ValueInt16;
					data.CrowdType = indexDataID;
					data.TargetID = static_cast<uint16>(bestTargetID);
					data.NewTargetLocation = targetLocations[bestTargetID];
					FInstancedStruct payload = FInstancedStruct::Make(data);
					
					OnManagerActionChange->ExecuteIfBound(payload);
				}
				
				if (!targetLocations.IsValidIndex(entityDataTargetID[i].ValueInt16)) continue;
				
				entityDataTargetLocation[i] = targetLocations[entityDataTargetID[i].ValueInt16];
			}
			
			indexDataID++;
		}
	}
};
