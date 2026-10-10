#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "FluxPrimeSpatialGridSystem.h"
#include "NavigationSystem.h"
#include "Detour/DetourNavMeshQuery.h"
#include "DetourCrowd/DetourPathCorridor.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperBool.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "NavMesh/RecastNavMesh.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Generics/Data/FluxPrimeNavigationData.h"
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
struct FFluxPrimeNavigationSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
	dtPathCorridor Corridor;
	dtNavMeshQuery* NavQuery = nullptr;
	dtQueryFilter Filter;
	
	UPROPERTY()
	UNavigationSystemV1* NavSys = nullptr;
	
	UPROPERTY()
	ARecastNavMesh* RecastNavMesh = nullptr;
	
	UPROPERTY()
	ANavigationData* NavData = nullptr;
	FSharedConstNavQueryFilter NavFilter;
	
	UPROPERTY(EditAnywhere)
	int32 QueuePathCountPerFrame = 20;
	UPROPERTY(EditAnywhere)
	int32 QueueCorridorCountPerFrame = 20;
	
	UPROPERTY(EditAnywhere)
	float OptimizeTimeSameCellID = 0.15f;
	UPROPERTY(EditAnywhere)
	float OptimizeTimeDifferenceCellID = 0.5f;
	
	UPROPERTY(EditAnywhere)
	float MoveTargetTimeSameCellID = 0.25f;
	UPROPERTY(EditAnywhere)
	float MoveTargetTimeDifferenceCellID = 1.5f;
	
	UPROPERTY(EditAnywhere)
	float RandomCircleRadius = 150.0f;
	
	UPROPERTY(EditAnywhere)
	FFluxPrimeSpatialGridSystem SpatialGridSystems;
	
	UPROPERTY(EditAnywhere)
	FNavigationSystemDataTagConfig DataTagConfig;

	TArray<int16> EntityDataCurrentLocationID, EntityDataPreviousLocationID, EntityDataSizeID, EntityDataCellIdID, EntityDataCorridorsID, EntityDataTargetLocationID, EntityDataCurrentWaypointLocationID, EntityDataLastReplanLocationID, EntityDataLastMoveLocationID, EntityDataWaitTimeID, EntityDataLastOptimizeTimeID, EntityDataLastMoveTargetTimeID, EntityDataCountCorridorID, EntityDataNeedReplanID, EntityDataWaypointsID, EntityDataCountWaypointsID;
	TArray<FFluxPrimeNavigationDataRequest> EntityPathQueue;
	TArray<FFluxPrimeNavigationDataRequest> EntityCorridorQueue;
	
private:
	void UnrealToRecast(const FVector& UnrealPoint, dtReal RecastPoint[3])
	{
		RecastPoint[0] = -UnrealPoint.X;
		RecastPoint[1] =  UnrealPoint.Z;
		RecastPoint[2] = -UnrealPoint.Y;
	}

	FVector RecastToUnreal(const dtReal* RecastPoint)
	{
		return FVector(-RecastPoint[0], -RecastPoint[2], RecastPoint[1]);
	}
	
	bool FindStraightPath(const FVector3d& startPos, const FVector3d& endPos,
		FFluxPrimeCrowdsWaypoint& outPath, uint8& outCountPath, const int options = DT_STRAIGHTPATH_AREA_CROSSINGS)
	{
		if ( GetPathCount() <= 0)return false;

		dtQueryResult result;

		dtReal dtStart[3];
		dtReal dtEnd[3];
		
		UnrealToRecast(startPos, dtStart);
		UnrealToRecast(endPos, dtEnd);

		const dtStatus Status = NavQuery->findStraightPath(
			dtStart,
			dtEnd,
			GetPath(),
			GetPathCount(),
			result,
			options);

		if (dtStatusFailed(Status)) return false;

		outCountPath = static_cast<int8>(result.size());

		for (int32 i = 0; i < outCountPath; ++i)
		{
			const dtReal* Pos = result.getPos(i);

			outPath.Waypoints[i] = RecastToUnreal(Pos);
			outPath.PolyRefs[i] = result.getRef(i);
			outPath.Flags[i] = result.getFlag(i);
		}
		
		return true;
	}
	
	bool FindPath(const FVector3d& startNearestPoint, const FVector3d& endNearestPoint, 
		FFluxPrimeCrowdsCorridor& OutCorridor, uint8& OutCorridorCount)
	{
		FPathFindingQuery Query(
			nullptr,
			*NavData,
			startNearestPoint,
			endNearestPoint
		);

		FPathFindingResult result = NavSys->FindPathSync(Query);
		
		if (!result.IsSuccessful()) return false;
		
		const FNavMeshPath* NavMeshPath = static_cast<const FNavMeshPath*>(result.Path.Get());
		OutCorridorCount = FMath::Min(NavMeshPath->PathCorridor.Num(), FluxConfig::CorridorsCount);
		FMemory::Memcpy(OutCorridor.Corridors.GetData(), NavMeshPath->PathCorridor.GetData(), sizeof(NavNodeRef) * OutCorridorCount);

		return true;
	}
	
	bool FindNearestPoly(const FVector3d& position, const FVector3d& searchExtent, NavNodeRef& outPoly, FVector3d& outNearestPoint)
	{
		NavNodeRef polyRef = RecastNavMesh->FindNearestPoly(
			position,
			searchExtent,
			NavFilter,
			nullptr);
		
		FNavLocation navLocation;
		bool bFound = RecastNavMesh->ProjectPoint(
			position,
			navLocation,
			searchExtent,
			NavFilter);
		
		if (bFound)
		{
			outPoly = polyRef; 
			outNearestPoint = navLocation.Location;
		}
		
		return bFound;
	}
	
	bool MovePosition(const FVector& newPosition)
	{
		dtReal dtPos[3];
		UnrealToRecast(newPosition, dtPos);
		return Corridor.movePosition(dtPos, NavQuery, &Filter);
	}
	
	bool MoveTargetPosition(const FVector& newTarget)
	{
		dtReal dtTarget[3];
		UnrealToRecast(newTarget, dtTarget);

		Corridor.moveTargetPosition(dtTarget, NavQuery, &Filter);
		return true;
	}
	
	bool OptimizePathTopology()
	{
		return Corridor.optimizePathTopology(NavQuery, &Filter);
	}
	
	void Reset(NavNodeRef startPoly, const FVector& startLocation)
	{
		dtReal dtStart[3];
		UnrealToRecast(startLocation, dtStart);
		Corridor.reset(startPoly, dtStart);
	}
	
	void SetCorridor(const FVector& target, const TArray<NavNodeRef>& corridorPath)
	{
		dtReal dtTarget[3];
		UnrealToRecast(target, dtTarget);
		Corridor.setCorridor(dtTarget, corridorPath.GetData(), corridorPath.Num());
	}
	
	const dtPolyRef* GetPath() const
	{
		return Corridor.getPath();
	}

	int32 GetPathCount() const
	{
		return Corridor.getPathCount();
	}
	
	void Reset(const FFluxPrimeCrowdsCorridor& corridor, const FVector& currentLocation)
	{
		dtReal dtStart[3];
		UnrealToRecast(currentLocation, dtStart);
		NavNodeRef startPoly = corridor.Corridors[0];
		Corridor.reset(startPoly, dtStart);
	}
	
	float SelectFloat(bool condition, float a, float b) const
	{
		return (condition)? a : b;
	}
	
	void Load(const FFluxPrimeCrowdsCorridor& corridor, uint8 corridorCount, const FVector& targetLocation)
	{
		dtReal dtTarget[3];
		UnrealToRecast(targetLocation, dtTarget);
		Corridor.setCorridor(dtTarget, corridor.Corridors.GetData(), corridorCount);
	}
	
	void Save(FFluxPrimeCrowdsCorridor& corridor, uint8& corridorCount)
	{
		corridorCount = FMath::Min(GetPathCount(), FluxConfig::CorridorsCount);

		const dtPolyRef* path = GetPath();

		for (int32 i = 0; i < corridorCount; ++i)
		{
			corridor.Corridors[i] = path[i];
		}
	}
	
	void QueueFindPath(bool& needReplan, FVector& location, FVector& targetLocation, 
		FVector& lastReplanLocation, FVector& lastMoveLocation, int16& size, FFluxPrimeCrowdsCorridor& outCorridor, uint8& outCorridorCount)
	{
		size = FMath::Max(.1f, size);
		NavNodeRef startPoly, targetPoly;
		FVector3d startNearestPoint, targetNearestPoint;
		FVector searchExtent( size, size, size * 2.f);
		FVector currentTargetLocation = targetLocation + FVector(FMath::RandPointInCircle(RandomCircleRadius), 0.0f);
		
		bool bFindNearestPoly = FindNearestPoly(location, searchExtent, startPoly, startNearestPoint) 
			&& FindNearestPoly(currentTargetLocation, searchExtent, targetPoly, targetNearestPoint);
		
		if (!bFindNearestPoly) return;
		
		if (!FindPath(startNearestPoint, targetNearestPoint, outCorridor, outCorridorCount))
		{
			outCorridorCount = 0;
			return;
		}
		
		lastReplanLocation = currentTargetLocation;
		lastMoveLocation = lastReplanLocation;
		
		Load(outCorridor, outCorridorCount, lastReplanLocation);
		Save(outCorridor, outCorridorCount);
		
		needReplan = false;
	}
	
	void QueueCorridor(FFluxPrimeCrowdsCorridor& corridors, uint8& countCorridor, FVector& location, FVector& previousLocation,
		FVector& targetLocation, FVector& lastMoveTargetLocation, FFluxPrimeCrowdsWaypoint& waypoints, uint8& countWaypoints, 
		int32& cellID, float& currentTime, float& lastOptimizeTime, float& lastMoveTargetTime)
	{
		int32 targetCellID = SpatialGridSystems.GetSpatialGridSystemsCellID(lastMoveTargetLocation);
		
		if (countCorridor == 0)
		{
			countWaypoints = 0;
			return;
		}
		
		Reset(corridors, previousLocation);
		Load(corridors, countCorridor, lastMoveTargetLocation);
		
		if (!MovePosition(location)) return;
		
		if (currentTime - lastOptimizeTime > SelectFloat(cellID == targetCellID, OptimizeTimeSameCellID, OptimizeTimeDifferenceCellID))
		{
			if (!OptimizePathTopology()) return;
			lastOptimizeTime = currentTime;
		}
		
		if (currentTime - lastMoveTargetTime > SelectFloat(cellID == targetCellID, MoveTargetTimeSameCellID, MoveTargetTimeDifferenceCellID))
		{
			lastMoveTargetTime = currentTime;
			lastMoveTargetLocation = targetLocation;
		}
		
		if (!FindStraightPath(location, lastMoveTargetLocation, 
			waypoints,countWaypoints)) countWaypoints = 0;
		
		Save(corridors, countCorridor);
		
		previousLocation = location;
	}
	
public:
	void Initialized(FFluxPrimeNavigationSystemsContext context)
	{
		check(context.world);

		World = context.world;
		
		SpatialGridSystems.Initialized(*context.dataStores);
		
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

		SpatialGridSystems.UpdateSpatialGridSystem(dataStores);
		
		EntityPathQueue.Reset();
		EntityCorridorQueue.Reset();
		float currentTime = World->GetTimeSeconds();
		
		int16 indexDataID = 0;
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
				targetCellId = SpatialGridSystems.GetSpatialGridSystemsCellID(entityDataTargetLocation[i]);
				lastReplanTargetCellId = SpatialGridSystems.GetSpatialGridSystemsCellID(entityDataLastReplanLocation[i]);
				
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
					EntityPathQueue.Add(tempPathRequest);
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
					EntityCorridorQueue.Add(tempPathRequest);
				}
			}
			
			indexDataID++;
		}
		
		int32 indexInternalData = 0;
		
		EntityPathQueue.Sort();
		int32 queuePathLimit = FMath::Min(QueuePathCountPerFrame, EntityPathQueue.Num());
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
		int32 queueCorridorLimit = FMath::Min(QueueCorridorCountPerFrame, EntityCorridorQueue.Num());
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