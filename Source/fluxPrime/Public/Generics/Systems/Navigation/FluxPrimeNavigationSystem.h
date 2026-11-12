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
	FFluxPrimeDataConfig EntityDataCurrentLocationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataPreviousLocationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataSizeConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataCellIdConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataCorridorsConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataTargetLocationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataCurrentWaypointLocationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLastReplanLocationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLastMoveLocationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataWaitTimeConfig;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLastOptimizeTimeConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLastMoveTargetTimeConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataCountCorridorConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataNeedReplanConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataWaypointsConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataCountWaypointsConfig;

    bool IsValid() const
    {
    	return EntityDataCurrentLocationConfig.EntityDataTag.IsValid() 
		  && EntityDataPreviousLocationConfig.EntityDataTag.IsValid() 
		  && EntityDataSizeConfig.EntityDataTag.IsValid() 
		  && EntityDataCellIdConfig.EntityDataTag.IsValid() 
		  && EntityDataCorridorsConfig.EntityDataTag.IsValid() 
		  && EntityDataTargetLocationConfig.EntityDataTag.IsValid() 
		  && EntityDataCurrentWaypointLocationConfig.EntityDataTag.IsValid() 
		  && EntityDataLastReplanLocationConfig.EntityDataTag.IsValid() 
		  && EntityDataLastMoveLocationConfig.EntityDataTag.IsValid()
		  && EntityDataWaitTimeConfig.EntityDataTag.IsValid()
		  && EntityDataLastOptimizeTimeConfig.EntityDataTag.IsValid() 
		  && EntityDataLastMoveTargetTimeConfig.EntityDataTag.IsValid() 
		  && EntityDataCountCorridorConfig.EntityDataTag.IsValid() 
		  && EntityDataNeedReplanConfig.EntityDataTag.IsValid() 
		  && EntityDataWaypointsConfig.EntityDataTag.IsValid() 
		  && EntityDataCountWaypointsConfig.EntityDataTag.IsValid();
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
		
		int16 countData = context.dataStores->Num();
		EntityDataCurrentLocationID.Init(0, countData);
		EntityDataPreviousLocationID.Init(0, countData);
		EntityDataSizeID.Init(0, countData);
		EntityDataCellIdID.Init(0, countData);
		EntityDataCorridorsID.Init(0, countData);
		EntityDataTargetLocationID.Init(0, countData);
		EntityDataCurrentWaypointLocationID.Init(0, countData);
		EntityDataLastReplanLocationID.Init(0, countData);
		EntityDataLastMoveLocationID.Init(0, countData);
		EntityDataWaitTimeID.Init(0, countData);
		EntityDataLastOptimizeTimeID.Init(0, countData);
		EntityDataLastMoveTargetTimeID.Init(0, countData);
		EntityDataCountCorridorID.Init(0, countData);
		EntityDataNeedReplanID.Init(0, countData);
		EntityDataWaypointsID.Init(0, countData);
		EntityDataCountWaypointsID.Init(0, countData);
		
		int32 dataTotalCount = 0;
		int32 indexDataId = 0;
		for (auto& dataStore : *context.dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataCurrentLocationConfig, EntityDataCurrentLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataPreviousLocationConfig, EntityDataPreviousLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataSizeConfig, EntityDataSizeID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataCellIdConfig, EntityDataCellIdID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataCorridorsConfig, EntityDataCorridorsID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataTargetLocationConfig, EntityDataTargetLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataCurrentWaypointLocationConfig, EntityDataCurrentWaypointLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataLastReplanLocationConfig, EntityDataLastReplanLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataLastMoveLocationConfig, EntityDataLastMoveLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataWaitTimeConfig, EntityDataWaitTimeID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataLastOptimizeTimeConfig, EntityDataLastOptimizeTimeID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataLastMoveTargetTimeConfig, EntityDataLastMoveTargetTimeID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataCountCorridorConfig, EntityDataCountCorridorID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataNeedReplanConfig, EntityDataNeedReplanID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataWaypointsConfig, EntityDataWaypointsID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataCountWaypointsConfig, EntityDataCountWaypointsID[indexDataId]);
			
			dataTotalCount += dataStore.GetTotalEntityCount();
			indexDataId++;
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

		if (!IsAllDataValid)
		{
			UE_LOG(LogTemp, Error, TEXT("NAVIGATION SYSTEM :: SOME DATA NOT VALID"));
			return;
		}
		
		SpatialGridSystem.UpdateSpatialGridSystem(dataStores);
		
		float currentTime = World->GetTimeSeconds();
		
		int32 indexDataID = 0;
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
			FFluxPrimeCrowdsCorridor* entityDataCorridors = reinterpret_cast<FFluxPrimeCrowdsCorridor*>(dataStore.GetRawBufferData(EntityDataCorridorsID[indexDataID]));
			FFluxPrimeWrapperUint8* entityDataCountCorridor = reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountCorridorID[indexDataID]));
			FFluxPrimeWrapperBool* entityDataNeedReplan = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataNeedReplanID[indexDataID]));
			FFluxPrimeCrowdsWaypoint* entityDataWaypoints = reinterpret_cast<FFluxPrimeCrowdsWaypoint*>(dataStore.GetRawBufferData(EntityDataWaypointsID[indexDataID]));
			FFluxPrimeWrapperUint8* entityDataCountWaypoints = reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountWaypointsID[indexDataID]));
			
			check(entityDataSize && entityDataCellId && entityDataCurrentTargetLocation && entityDataCurrentLocation);
			check(entityDataPreviousLocation && entityDataTargetLocation && entityDataLastReplanLocation && entityDataLastMoveLocation);
			check(entityDataWaitTime && entityDataLastOptimizeTime && entityDataLastMoveTargetTime && entityDataCorridors);
			check(entityDataCountCorridor && entityDataNeedReplan && entityDataWaypoints && entityDataCountWaypoints);
			
			for (int i = 0; i < dataStore.GetActiveEntityCount(); ++i)
			{
				targetCellId = SpatialGridSystem.GetSpatialGridSystemsCellID(entityDataTargetLocation[i]);
				lastReplanTargetCellId = SpatialGridSystem.GetSpatialGridSystemsCellID(entityDataLastReplanLocation[i]);
				
				entityDataNeedReplan[i].ValueBool = (targetCellId != lastReplanTargetCellId) || entityDataNeedReplan[i].ValueBool;
				
				entityDataWaitTime[i].ValueFloat += deltaTime;

				const float arrivalRadiusSq = FMath::Square(FMath::Max(60.0f, entityDataSize[i].ValueInt16 * 1.5f));
				const float distToWaypointSq = FVector::DistSquaredXY(entityDataCurrentLocation[i], entityDataCurrentTargetLocation[i]);
				const bool bReachedCurrentWaypoint = (distToWaypointSq <= arrivalRadiusSq);

				if (bReachedCurrentWaypoint && !entityDataNeedReplan[i].ValueBool)
				{
					if (entityDataCountWaypoints[i].ValueUint8 > 2 &&
						FVector::DistSquaredXY(entityDataCurrentTargetLocation[i], entityDataWaypoints[i].Waypoints[1]) <= 1.0f)
						entityDataCurrentTargetLocation[i] = entityDataWaypoints[i].Waypoints[2];
					else
						entityDataCurrentTargetLocation[i] = entityDataCurrentLocation[i];
				}

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

				float baseScore = (targetCellId == entityDataCellId[i].ValueInt32) ? 5.0f : 0.0f;
				float timeScore = entityDataWaitTime[i].ValueFloat * 10.0f;

				if (entityDataNeedReplan[i].ValueBool)
				{
					if (entityDataCountWaypoints[i].ValueUint8 <= 1)
						entityDataCurrentTargetLocation[i] = entityDataCurrentLocation[i];

					FFluxPrimeNavigationDataRequest tempPathRequest;
					tempPathRequest.PriorityScore = baseScore + timeScore;
					tempPathRequest.DataStoreIndex = indexDataID;
					tempPathRequest.InternalIndex = i;
					tempPathRequest.DataTask = tempPathTask;

					EntityPathQueue[countPathQueue] = tempPathRequest;
					countPathQueue++;
				}
				else
				{
					float arrivalBoost = bReachedCurrentWaypoint ? 50.0f : 0.0f;

					FFluxPrimeNavigationDataRequest tempCorridorRequest;
					tempCorridorRequest.PriorityScore = baseScore + timeScore + arrivalBoost;
					tempCorridorRequest.DataStoreIndex = indexDataID;
					tempCorridorRequest.InternalIndex = i;
					tempCorridorRequest.DataTask = tempPathTask;

					EntityCorridorQueue[countCorridorQueue] = tempCorridorRequest;
					countCorridorQueue++;
				}
			}
			
			indexDataID++;
		}
		
		int32 indexInternalData = 0;
		
		if (countPathQueue > 1)
			Algo::Sort(TArrayView<FFluxPrimeNavigationDataRequest>(EntityPathQueue.GetData(), countPathQueue));

		int32 queuePathLimit = FMath::Min(QueuePathCountPerFrame, countPathQueue);
		for (int i = 0; i < queuePathLimit; ++i)
		{
			auto& data = EntityPathQueue[i];
			auto& task = data.DataTask;
			
			indexDataID = data.DataStoreIndex;
			indexInternalData = data.InternalIndex;
			FFluxPrimeArchetypeDataStore& dataStore = dataStores[indexDataID];
			
			QueueFindPath(task.EntityDataNeedReplan, task.EntityDataCurrentLocation, task.EntityDataPreviousLocation,
						task.EntityDataTargetLocation, task.EntityDataLastReplanLocation, task.EntityDataLastMoveLocation, 
						task.EntityDataSize, task.EntityDataCorridors, task.EntityDataCountCorridor,
						task.EntityDataWaypoints, task.EntityDataCountWaypoints);
        
			reinterpret_cast<FFluxPrimeCrowdsCorridor*>(dataStore.GetRawBufferData(EntityDataCorridorsID[indexDataID]))[indexInternalData] = task.EntityDataCorridors;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataPreviousLocationID[indexDataID]))[indexInternalData] = task.EntityDataPreviousLocation;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]))[indexInternalData] = task.EntityDataTargetLocation;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastReplanLocationID[indexDataID]))[indexInternalData] = task.EntityDataLastReplanLocation;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastMoveLocationID[indexDataID]))[indexInternalData] = task.EntityDataLastMoveLocation;
			reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountCorridorID[indexDataID]))[indexInternalData].ValueUint8 = task.EntityDataCountCorridor;
			reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataNeedReplanID[indexDataID]))[indexInternalData].ValueBool = task.EntityDataNeedReplan;
			reinterpret_cast<FFluxPrimeCrowdsWaypoint*>(dataStore.GetRawBufferData(EntityDataWaypointsID[indexDataID]))[indexInternalData] = task.EntityDataWaypoints;
			reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountWaypointsID[indexDataID]))[indexInternalData].ValueUint8 = task.EntityDataCountWaypoints;
		
			reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataWaitTimeID[indexDataID]))[indexInternalData].ValueFloat = 0.0f;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointLocationID[indexDataID]))[indexInternalData] = (task.EntityDataCountWaypoints > 1) ? task.EntityDataWaypoints.Waypoints[1] : task.EntityDataCurrentLocation;
		}
		
		if (countCorridorQueue > 1)
			Algo::Sort(TArrayView<FFluxPrimeNavigationDataRequest>(EntityCorridorQueue.GetData(), countCorridorQueue));

		int32 queueCorridorLimit = FMath::Min(QueueCorridorCountPerFrame, countCorridorQueue);
		for (int i = 0; i < queueCorridorLimit; ++i)
		{
			auto& data = EntityCorridorQueue[i];
			auto& task = data.DataTask;
			
			indexDataID = data.DataStoreIndex;
			indexInternalData = data.InternalIndex;
			FFluxPrimeArchetypeDataStore& dataStore = dataStores[indexDataID];
			
			QueueCorridor(task.EntityDataNeedReplan, task.EntityDataCorridors, task.EntityDataCountCorridor, 
						task.EntityDataCurrentLocation, task.EntityDataPreviousLocation,
						task.EntityDataTargetLocation, task.EntityDataLastMoveLocation, task.EntityDataWaypoints, task.EntityDataCountWaypoints,
						task.EntityDataCellId, currentTime, task.EntityDataLastOptimizeTime, task.EntityDataLastMoveTargetTime);
        
			reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataNeedReplanID[indexDataID]))[indexInternalData].ValueBool = task.EntityDataNeedReplan;
			reinterpret_cast<FFluxPrimeCrowdsCorridor*>(dataStore.GetRawBufferData(EntityDataCorridorsID[indexDataID]))[indexInternalData] = task.EntityDataCorridors;
			reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountCorridorID[indexDataID]))[indexInternalData].ValueUint8 = task.EntityDataCountCorridor;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataPreviousLocationID[indexDataID]))[indexInternalData] = task.EntityDataPreviousLocation;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastMoveLocationID[indexDataID]))[indexInternalData] = task.EntityDataLastMoveLocation;
			reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataLastOptimizeTimeID[indexDataID]))[indexInternalData].ValueFloat = task.EntityDataLastOptimizeTime;
			reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataLastMoveTargetTimeID[indexDataID]))[indexInternalData].ValueFloat = task.EntityDataLastMoveTargetTime;
			reinterpret_cast<FFluxPrimeCrowdsWaypoint*>(dataStore.GetRawBufferData(EntityDataWaypointsID[indexDataID]))[indexInternalData] = task.EntityDataWaypoints;
			reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountWaypointsID[indexDataID]))[indexInternalData].ValueUint8 = task.EntityDataCountWaypoints;
	
			reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataWaitTimeID[indexDataID]))[indexInternalData].ValueFloat = 0.0f;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointLocationID[indexDataID]))[indexInternalData] = (task.EntityDataCountWaypoints > 1) ? task.EntityDataWaypoints.Waypoints[1] : task.EntityDataCurrentLocation;
		}
	}
};