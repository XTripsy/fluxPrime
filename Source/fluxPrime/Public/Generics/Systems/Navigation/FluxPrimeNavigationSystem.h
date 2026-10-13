#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperBool.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Generics/Data/FluxPrimeNavigationData.h"
#include "FluxPrimeBaseNavigationSystem.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeNavigationSystem.generated.h"

USTRUCT(BlueprintType)
struct FNavigationSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataCurrentLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataPreviousLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataSizeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataCellIdTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataCorridorsTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataTargetLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataCurrentWaypointLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLastReplanLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLastMoveLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataWaitTimeTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLastOptimizeTimeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLastMoveTargetTimeTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataCountCorridorTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataNeedReplanTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataWaypointsTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataCountWaypointsTag;

    bool IsValid() const
    {
       return EntityDataCurrentLocationTag.IsValid() 
          && EntityDataPreviousLocationTag.IsValid() 
          && EntityDataSizeTag.IsValid() 
          && EntityDataCellIdTag.IsValid() 
          && EntityDataCorridorsTag.IsValid() 
          && EntityDataTargetLocationTag.IsValid() 
          && EntityDataCurrentWaypointLocationTag.IsValid() 
          && EntityDataLastReplanLocationTag.IsValid() 
          && EntityDataLastMoveLocationTag.IsValid()
          && EntityDataWaitTimeTag.IsValid()
          && EntityDataLastOptimizeTimeTag.IsValid() 
          && EntityDataLastMoveTargetTimeTag.IsValid() 
          && EntityDataCountCorridorTag.IsValid() 
          && EntityDataNeedReplanTag.IsValid() 
          && EntityDataWaypointsTag.IsValid() 
          && EntityDataCountWaypointsTag.IsValid();
    }
};

USTRUCT(BlueprintType)
struct FFluxPrimeNavigationSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeNavigationSystem : public FFluxPrimeBaseNavigationSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FNavigationSystemDataTagConfig DataTagConfig;

	TArray<int16> EntityDataCurrentLocationID, EntityDataPreviousLocationID, EntityDataSizeID, EntityDataCellIdID, EntityDataCorridorsID, EntityDataTargetLocationID, EntityDataCurrentWaypointLocationID, EntityDataLastReplanLocationID, EntityDataLastMoveLocationID, EntityDataWaitTimeID, EntityDataLastOptimizeTimeID, EntityDataLastMoveTargetTimeID, EntityDataCountCorridorID, EntityDataNeedReplanID, EntityDataWaypointsID, EntityDataCountWaypointsID;
	TArray<FFluxPrimeNavigationDataRequest> EntityPathQueue;
	TArray<FFluxPrimeNavigationDataRequest> EntityCorridorQueue;
	
public:
	void Initialized(FFluxPrimeNavigationSystemsContext context)
	{
		check(context.world);

		World = context.world;
		
		SpatialGridSystem.Initialized(*context.dataStores);
		
		NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
		check(NavSys);

		NavData = NavSys->GetDefaultNavDataInstance();
		check(NavData);

		RecastNavMesh = Cast<ARecastNavMesh>(NavData);
		check(RecastNavMesh);
		
		NavQuery = dtAllocNavMeshQuery();
		check(NavQuery);

		dtNavMesh* DtNavMesh = RecastNavMesh->GetRecastMesh();
		check(DtNavMesh);

		NavQuery->init(DtNavMesh, 2048);
		Corridor.init(256);
		
		int16 dataCount = context.dataStores->Num();
		EntityDataCurrentLocationID.Reserve(dataCount);
		EntityDataPreviousLocationID.Reserve(dataCount);
		EntityDataSizeID.Reserve(dataCount);
		EntityDataCellIdID.Reserve(dataCount);
		EntityDataCorridorsID.Reserve(dataCount);
		EntityDataTargetLocationID.Reserve(dataCount);
		EntityDataCurrentWaypointLocationID.Reserve(dataCount);
		EntityDataLastReplanLocationID.Reserve(dataCount);
		EntityDataLastMoveLocationID.Reserve(dataCount);
		EntityDataWaitTimeID.Reserve(dataCount);
		EntityDataLastOptimizeTimeID.Reserve(dataCount);
		EntityDataLastMoveTargetTimeID.Reserve(dataCount);
		EntityDataCountCorridorID.Reserve(dataCount);
		EntityDataNeedReplanID.Reserve(dataCount);
		EntityDataWaypointsID.Reserve(dataCount);
		EntityDataCountWaypointsID.Reserve(dataCount);
		
		int32 dataTotalCount = 0;
		
		for (auto& dataStore : *context.dataStores)
		{
			EntityDataCurrentLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCurrentLocationTag));
			EntityDataPreviousLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataPreviousLocationTag));
			EntityDataSizeID.Add(dataStore.GetDataID(DataTagConfig.EntityDataSizeTag));
			EntityDataCellIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCellIdTag));
			EntityDataCorridorsID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCorridorsTag));
			EntityDataTargetLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataTargetLocationTag));
			EntityDataCurrentWaypointLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCurrentWaypointLocationTag));
			EntityDataLastReplanLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLastReplanLocationTag));
			EntityDataLastMoveLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLastMoveLocationTag));
			EntityDataWaitTimeID.Add(dataStore.GetDataID(DataTagConfig.EntityDataWaitTimeTag));
			EntityDataLastOptimizeTimeID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLastOptimizeTimeTag));
			EntityDataLastMoveTargetTimeID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLastMoveTargetTimeTag));
			EntityDataCountCorridorID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCountCorridorTag));
			EntityDataNeedReplanID.Add(dataStore.GetDataID(DataTagConfig.EntityDataNeedReplanTag));
			EntityDataWaypointsID.Add(dataStore.GetDataID(DataTagConfig.EntityDataWaypointsTag));
			EntityDataCountWaypointsID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCountWaypointsTag));
			
			dataTotalCount += dataStore.GetTotalEntityCount();
		}
		
		EntityPathQueue.Init(FFluxPrimeNavigationDataRequest(), dataTotalCount);
		EntityCorridorQueue.Init(FFluxPrimeNavigationDataRequest(), dataTotalCount);
	}
	
	void SyncNewTarget(FFluxPrimeArchetypeDataStore& dataStore, int32 indexDataID, int32 indexData, bool newRequestNeedReplan)
	{
		reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataNeedReplanID[indexDataID]))[indexData].ValueBool = newRequestNeedReplan;
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_Systems);

		SpatialGridSystem.UpdateSpatialGridSystem(dataStores);
		
		float currentTime = World->GetTimeSeconds();
		
		int32 indexDataID = 0;
		int32 indexData = 0;
		int32 countPathQueue = 0, countCorridorQueue = 0;
		int32 lastReplanTargetCellId = 0, targetCellId = 0;
		for (auto& dataStore : dataStores)
		{
			FFluxPrimeWrapperInt16* entityDataSize = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataSizeID[indexDataID]));
			FFluxPrimeWrapperInt32* entityDataCellId = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataCellIdID[indexDataID]));
			FVector* entityDataCurrentTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointLocationID[indexDataID]));
			FVector* entityDataCurrentLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentLocationID[indexDataID]));
			FVector* entityDataPreviousLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataPreviousLocationID[indexDataID]));
			FVector* entityDataTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]));
			FVector* entityDataLastReplanLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastReplanLocationID[indexDataID]));
			FVector* entityDataLastMoveLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastMoveLocationID[indexDataID]));
			FFluxPrimeWrapperFloat* entityDataWaitTime = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataWaitTimeID[indexDataID]));
			FFluxPrimeWrapperFloat* entityDataLastOptimizeTime = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataLastOptimizeTimeID[indexDataID]));
			FFluxPrimeWrapperFloat* entityDataLastMoveTargetTime = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataLastMoveTargetTimeID[indexDataID]));
			/*-> ini*/FFluxPrimeCrowdsCorridor* entityDataCorridors = reinterpret_cast<FFluxPrimeCrowdsCorridor*>(dataStore.GetRawBufferData(EntityDataCorridorsID[indexDataID]));
			FFluxPrimeWrapperUint8* entityDataCountCorridor = reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountCorridorID[indexDataID]));
			FFluxPrimeWrapperBool* entityDataNeedReplan = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataNeedReplanID[indexDataID]));
			/*-> ini*/FFluxPrimeCrowdsWaypoint* entityDataWaypoints = reinterpret_cast<FFluxPrimeCrowdsWaypoint*>(dataStore.GetRawBufferData(EntityDataWaypointsID[indexDataID]));
			FFluxPrimeWrapperUint8* entityDataCountWaypoints = reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountWaypointsID[indexDataID]));
			
			check(entityDataSize);
			check(entityDataCellId);
			check(entityDataCurrentTargetLocation);
			check(entityDataCurrentLocation);
			check(entityDataPreviousLocation);
			check(entityDataTargetLocation);
			check(entityDataLastReplanLocation);
			check(entityDataLastMoveLocation);
			check(entityDataWaitTime);
			check(entityDataLastOptimizeTime);
			check(entityDataLastMoveTargetTime);
			check(entityDataCorridors);
			check(entityDataCountCorridor);
			check(entityDataNeedReplan);
			check(entityDataWaypoints);
			check(entityDataCountWaypoints);
			
			for (int i = 0; i < dataStore.GetActiveEntityCount(); ++i)
			{
				targetCellId = SpatialGridSystem.GetSpatialGridSystemsCellID(entityDataTargetLocation[i]);
				lastReplanTargetCellId = SpatialGridSystem.GetSpatialGridSystemsCellID(entityDataLastReplanLocation[i]);
				
				entityDataNeedReplan[i].ValueBool = targetCellId != lastReplanTargetCellId || entityDataNeedReplan[i].ValueBool;
				
				if (entityDataNeedReplan[i].ValueBool)
				{
					entityDataWaitTime[i].ValueFloat += currentTime;
					
					float baseScore = (targetCellId == entityDataCellId[i].ValueInt32) ? 5.0f : 0.0f;
					float timeScore = entityDataWaitTime[i].ValueFloat * 10.0f;
					
					FFluxPrimeNavigationTask tempPathTask;
					tempPathTask.EntityDataCurrentTargetLocation = entityDataCurrentTargetLocation[i];
					tempPathTask.EntityDataCellId = entityDataCellId[i].ValueInt32;
					tempPathTask.EntityDataCurrentLocation = entityDataCurrentLocation[i];
					tempPathTask.EntityDataSize = entityDataSize[i].ValueInt16;
					tempPathTask.EntityDataWaitTime = entityDataWaitTime[i].ValueFloat;
					tempPathTask.EntityDataPreviousLocation = entityDataPreviousLocation[i];
					tempPathTask.EntityDataCorridors = entityDataCorridors[i];
					tempPathTask.EntityDataTargetLocation = entityDataTargetLocation[i]; 
					tempPathTask.EntityDataLastReplanLocation = entityDataLastReplanLocation[i];
					tempPathTask.EntityDataLastMoveLocation = entityDataLastMoveLocation[i];
					tempPathTask.EntityDataLastOptimizeTime = entityDataLastOptimizeTime[i].ValueFloat;
					tempPathTask.EntityDataLastMoveTargetTime = entityDataLastMoveTargetTime[i].ValueFloat;
					tempPathTask.EntityDataCountCorridor = entityDataCountCorridor[i].ValueUint8;
					tempPathTask.EntityDataNeedReplan = entityDataNeedReplan[i].ValueBool;
					tempPathTask.EntityDataWaypoints = entityDataWaypoints[i];
					tempPathTask.EntityDataCountWaypoints = entityDataCountWaypoints[i].ValueUint8;
					
					FFluxPrimeNavigationDataRequest tempPathRequest;
					tempPathRequest.PriorityScore = baseScore + timeScore;
					tempPathRequest.DataStoreIndex = indexDataID;
					tempPathRequest.InternalIndex = i;
					tempPathRequest.DataTask = tempPathTask;
					EntityPathQueue[indexData] = tempPathRequest;
					
					countPathQueue++;
				}
				else
				{
					float baseScore = (targetCellId == entityDataCellId[i].ValueInt32) ? 5.0f : 0.0f;
					float timeScore = entityDataWaitTime[i].ValueFloat * 10.0f;
					
					FFluxPrimeNavigationTask tempPathTask;
					tempPathTask.EntityDataCurrentTargetLocation = entityDataCurrentTargetLocation[i];
					tempPathTask.EntityDataCellId = entityDataCellId[i].ValueInt32;
					tempPathTask.EntityDataCurrentLocation = entityDataCurrentLocation[i];
					tempPathTask.EntityDataSize = entityDataSize[i].ValueInt16;
					tempPathTask.EntityDataWaitTime = entityDataWaitTime[i].ValueFloat;
					tempPathTask.EntityDataPreviousLocation = entityDataPreviousLocation[i];
					tempPathTask.EntityDataCorridors = entityDataCorridors[i];
					tempPathTask.EntityDataTargetLocation = entityDataTargetLocation[i]; 
					tempPathTask.EntityDataLastReplanLocation = entityDataLastReplanLocation[i];
					tempPathTask.EntityDataLastMoveLocation = entityDataLastMoveLocation[i];
					tempPathTask.EntityDataLastOptimizeTime = entityDataLastOptimizeTime[i].ValueFloat;
					tempPathTask.EntityDataLastMoveTargetTime = entityDataLastMoveTargetTime[i].ValueFloat;
					tempPathTask.EntityDataCountCorridor = entityDataCountCorridor[i].ValueUint8;
					tempPathTask.EntityDataNeedReplan = entityDataNeedReplan[i].ValueBool;
					tempPathTask.EntityDataWaypoints = entityDataWaypoints[i];
					tempPathTask.EntityDataCountWaypoints = entityDataCountWaypoints[i].ValueUint8;
					
					FFluxPrimeNavigationDataRequest tempPathRequest;
					tempPathRequest.PriorityScore = baseScore + timeScore;
					tempPathRequest.DataStoreIndex = indexDataID;
					tempPathRequest.InternalIndex = i;
					tempPathRequest.DataTask = tempPathTask;
					EntityCorridorQueue[indexData] = tempPathRequest;
					
					countCorridorQueue++;
				}
				
				indexData++;
			}
			
			indexDataID++;
		}
		
		int32 indexInternalData = 0;
		
		EntityPathQueue.Sort();
		int32 queuePathLimit = FMath::Min(QueuePathCountPerFrame, countPathQueue);
		for (int i = 0; i < queuePathLimit; ++i)
		{
			auto& data = EntityPathQueue[i];
			auto& task = data.DataTask;
			
			indexDataID = data.DataStoreIndex;
			indexInternalData = data.InternalIndex;
			FFluxPrimeArchetypeDataStore& dataStore = dataStores[indexDataID];
			
			QueueFindPath(task.EntityDataNeedReplan, task.EntityDataCurrentLocation, task.EntityDataTargetLocation, 
						task.EntityDataLastReplanLocation, task.EntityDataLastMoveLocation, task.EntityDataSize,task.EntityDataCorridors, task.EntityDataCountCorridor);
        
			reinterpret_cast<FFluxPrimeCrowdsCorridor*>(dataStore.GetRawBufferData(EntityDataCorridorsID[indexDataID]))[indexInternalData] = task.EntityDataCorridors;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]))[indexInternalData] = task.EntityDataTargetLocation;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastReplanLocationID[indexDataID]))[indexInternalData] = task.EntityDataLastReplanLocation;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastMoveLocationID[indexDataID]))[indexInternalData] = task.EntityDataLastMoveLocation;
			reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountCorridorID[indexDataID]))[indexInternalData].ValueUint8 = task.EntityDataCountCorridor;
			reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataNeedReplanID[indexDataID]))[indexInternalData].ValueBool = task.EntityDataNeedReplan;
			
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointLocationID[indexDataID]))[indexInternalData] = (task.EntityDataCountWaypoints > 1)? task.EntityDataWaypoints.Waypoints[1] : task.EntityDataTargetLocation;
		}
		
		EntityCorridorQueue.Sort();
		int32 queueCorridorLimit = FMath::Min(QueueCorridorCountPerFrame, countCorridorQueue);
		for (int i = 0; i < queueCorridorLimit; ++i)
		{
			auto& data = EntityCorridorQueue[i];
			auto& task = data.DataTask;
			
			indexDataID = data.DataStoreIndex;
			indexInternalData = data.InternalIndex;
			FFluxPrimeArchetypeDataStore& dataStore = dataStores[indexDataID];
			
			QueueCorridor(task.EntityDataCorridors, task.EntityDataCountCorridor, task.EntityDataCurrentLocation, task.EntityDataPreviousLocation,
						task.EntityDataTargetLocation, task.EntityDataLastMoveLocation, task.EntityDataWaypoints, task.EntityDataCountWaypoints,
					task.EntityDataCellId, currentTime, task.EntityDataLastOptimizeTime, task.EntityDataLastMoveTargetTime);
        
			reinterpret_cast<FFluxPrimeCrowdsCorridor*>(dataStore.GetRawBufferData(EntityDataCorridorsID[indexDataID]))[indexInternalData] = task.EntityDataCorridors;
			reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountCorridorID[indexDataID]))[indexInternalData].ValueUint8 = task.EntityDataCountCorridor;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataPreviousLocationID[indexDataID]))[indexInternalData] = task.EntityDataPreviousLocation;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastMoveLocationID[indexDataID]))[indexInternalData] = task.EntityDataLastMoveLocation;
			reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataLastOptimizeTimeID[indexDataID]))[indexInternalData].ValueFloat = task.EntityDataLastOptimizeTime;
			reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataLastMoveTargetTimeID[indexDataID]))[indexInternalData].ValueFloat = task.EntityDataLastMoveTargetTime;
			reinterpret_cast<FFluxPrimeCrowdsWaypoint*>(dataStore.GetRawBufferData(EntityDataWaypointsID[indexDataID]))[indexInternalData] = task.EntityDataWaypoints;
			reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountWaypointsID[indexDataID]))[indexInternalData].ValueUint8 = task.EntityDataCountWaypoints;
			
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointLocationID[indexDataID]))[indexInternalData] = (task.EntityDataCountWaypoints > 1)? task.EntityDataWaypoints.Waypoints[1] : task.EntityDataTargetLocation;
		}
	}
};