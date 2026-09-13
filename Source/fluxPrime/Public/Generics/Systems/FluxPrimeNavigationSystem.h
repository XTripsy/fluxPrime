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
#if IF_WITH_EDITOR
#include "ProfilingDebugging/CpuProfilerTrace.h"
#endif
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
    FGameplayTag EntityDataCurrentTargetLocationTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataLastReplanLocationTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataLastMoveLocationTag;

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
          && EntityDataCurrentTargetLocationTag.IsValid() 
          && EntityDataLastReplanLocationTag.IsValid() 
          && EntityDataLastMoveLocationTag.IsValid() 
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
	uint16 QueuePathCountPerFrame = 20;
	UPROPERTY(EditAnywhere)
	uint16 QueueCorridorCountPerFrame = 20;
	
	UPROPERTY(EditAnywhere)
	float OptimizeTimeSameCellID = 0.15f;
	UPROPERTY(EditAnywhere)
	float OptimizeTimeDifferenceCellID = 0.5f;
	
	UPROPERTY(EditAnywhere)
	float MoveTargetTimeSameCellID = 0.25f;
	UPROPERTY(EditAnywhere)
	float MoveTargetTimeDifferenceCellID = 1.5f;
	
	UPROPERTY(EditAnywhere)
	FFluxPrimeSpatialGridSystem SpatialGridSystems;
	
	UPROPERTY(EditAnywhere)
	FNavigationSystemDataTagConfig DataTagConfig;

	TArray<int16> EntityDataCurrentLocationID, EntityDataPreviousLocationID, EntityDataSizeID, EntityDataCellIdID, EntityDataCorridorsID, EntityDataTargetLocationID, EntityDataCurrentTargetLocationID, EntityDataLastReplanLocationID, EntityDataLastMoveLocationID, EntityDataLastOptimizeTimeID, EntityDataLastMoveTargetTimeID, EntityDataCountCorridorID, EntityDataNeedReplanID, EntityDataWaypointsID, EntityDataCountWaypointsID;
	
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
	
	void QueueFindPath(uint16& queuePath, bool& needReplan, FVector& location, FVector& currentTargetLocation, 
		FVector& lastReplanLocation, FVector& lastMoveLocation, int16& size, FFluxPrimeCrowdsCorridor& outCorridor, uint8& outCorridorCount)
	{	
		if (queuePath > QueuePathCountPerFrame) return;
		
		size = FMath::Max(.1f, size);
		NavNodeRef startPoly, targetPoly;
		FVector3d startNearestPoint, targetNearestPoint;
		FVector searchExtent( size, size, size * 2.f);
		
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
		queuePath++;
	}
	
	void QueueCorridor(uint16& queueCorridor, FFluxPrimeCrowdsCorridor& corridors, uint8& countCorridor, FVector& location, FVector& previousLocation,
		FVector& targetLocation, FVector& lastMoveTargetLocation, FFluxPrimeCrowdsWaypoint& waypoints, uint8& countWaypoints, 
		int32& cellID, float& currentTime, float& lastOptimizeTime, float& lastMoveTargetTime, bool& needMoveTarget)
	{
		if (queueCorridor > QueueCorridorCountPerFrame) return;
		
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
		
		if (needMoveTarget || currentTime - lastMoveTargetTime > SelectFloat(cellID == targetCellID, MoveTargetTimeSameCellID, MoveTargetTimeDifferenceCellID))
		{
			needMoveTarget = !MoveTargetPosition(targetLocation);
			if (needMoveTarget) return;
			lastMoveTargetTime = currentTime;
			lastMoveTargetLocation = targetLocation;
		}
		
		if (!FindStraightPath(location, lastMoveTargetLocation, 
			waypoints,countWaypoints)) countWaypoints = 0;
		
		Save(corridors, countCorridor);
		
		previousLocation = location;
		
		queueCorridor++;
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
		
		for (auto& dataStore : *context.dataStores)
		{
			EntityDataCurrentLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCurrentLocationTag));
			EntityDataPreviousLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataPreviousLocationTag));
			EntityDataSizeID.Add(dataStore.GetDataID(DataTagConfig.EntityDataSizeTag));
			EntityDataCellIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCellIdTag));
			EntityDataCorridorsID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCorridorsTag));
			EntityDataTargetLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataTargetLocationTag));
			EntityDataCurrentTargetLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCurrentTargetLocationTag));
			EntityDataLastReplanLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLastReplanLocationTag));
			EntityDataLastMoveLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLastMoveLocationTag));
			EntityDataLastOptimizeTimeID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLastOptimizeTimeTag));
			EntityDataLastMoveTargetTimeID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLastMoveTargetTimeTag));
			EntityDataCountCorridorID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCountCorridorTag));
			EntityDataNeedReplanID.Add(dataStore.GetDataID(DataTagConfig.EntityDataNeedReplanTag));
			EntityDataWaypointsID.Add(dataStore.GetDataID(DataTagConfig.EntityDataWaypointsTag));
			EntityDataCountWaypointsID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCountWaypointsTag));
		}
	}
	
	void SyncNewTarget(FFluxPrimeArchetypeDataStore& dataStore, int32 indexDataID, int32 indexData, bool newRequestNeedReplan)
	{
		reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataNeedReplanID[indexDataID]))[indexData].ValueBool = newRequestNeedReplan;
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
#if IF_WITH_EDITOR
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_Systems);
#endif

		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			SpatialGridSystems.UpdateSpatialGridSystem(dataStore);
			
			float CurrentTime = World->GetTimeSeconds();
			
			FVector* entityDataCurrentLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentLocationID[indexDataID]));
			FVector* entityDataPreviousLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataPreviousLocationID[indexDataID]));
			FFluxPrimeWrapperInt16* entityDataSize = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataSizeID[indexDataID]));
			FFluxPrimeWrapperInt32* entityDataCellId = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataCellIdID[indexDataID]));
			FFluxPrimeCrowdsCorridor* entityDataCorridors = reinterpret_cast<FFluxPrimeCrowdsCorridor*>(dataStore.GetRawBufferData(EntityDataCorridorsID[indexDataID]));
			FVector* entityDataTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]));
			FVector* entityDataCurrentTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentTargetLocationID[indexDataID]));
			FVector* entityDataLastReplanLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastReplanLocationID[indexDataID]));
			FVector* entityDataLastMoveLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastMoveLocationID[indexDataID]));
			FFluxPrimeWrapperFloat* entityDataLastOptimizeTime = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataLastOptimizeTimeID[indexDataID]));
			FFluxPrimeWrapperFloat* entityDataLastMoveTargetTime = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataLastMoveTargetTimeID[indexDataID]));
			FFluxPrimeWrapperUint8* entityDataCountCorridor = reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountCorridorID[indexDataID]));
			FFluxPrimeWrapperBool* entityDataNeedReplan = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataNeedReplanID[indexDataID]));
			FFluxPrimeCrowdsWaypoint* entityDataWaypoints = reinterpret_cast<FFluxPrimeCrowdsWaypoint*>(dataStore.GetRawBufferData(EntityDataWaypointsID[indexDataID]));
			FFluxPrimeWrapperUint8* entityDataCountWaypoints = reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountWaypointsID[indexDataID]));
			
			uint16 queuePath = 0;
			uint16 queueCorridor = 0;
			bool needMoveTarget = false;
			for (int i = 0; i < dataStore.GetActiveEntityCount(); ++i)
			{
				needMoveTarget = false;
				
				const float distanceReplan = FVector::DistSquaredXY(entityDataLastReplanLocation[i], entityDataTargetLocation[i]);
				const float distanceMoveTarget = FVector::DistSquaredXY(entityDataLastMoveLocation[i], entityDataTargetLocation[i]);
				
				entityDataNeedReplan[i].ValueBool = distanceReplan > 1500 || entityDataNeedReplan[i].ValueBool;
				needMoveTarget = distanceMoveTarget > 400;
				
				if (entityDataNeedReplan[i].ValueBool)
				{
					QueueFindPath(queuePath, entityDataNeedReplan[i].ValueBool, entityDataCurrentLocation[i], entityDataTargetLocation[i], 
						entityDataLastReplanLocation[i], entityDataLastMoveLocation[i], entityDataSize[i].ValueInt16,entityDataCorridors[i], entityDataCountCorridor[i].ValueUint8);
				}
				else
				{
					QueueCorridor(queueCorridor, entityDataCorridors[i], entityDataCountCorridor[i].ValueUint8, entityDataCurrentLocation[i], entityDataPreviousLocation[i],
						entityDataTargetLocation[i], entityDataLastMoveLocation[i], entityDataWaypoints[i], entityDataCountWaypoints[i].ValueUint8,
					entityDataCellId[i].ValueInt32, CurrentTime, entityDataLastOptimizeTime[i].ValueFloat, entityDataLastMoveTargetTime[i].ValueFloat, needMoveTarget);
				}
				
				entityDataCurrentTargetLocation[i] = (entityDataCountWaypoints[i].ValueUint8 > 1)? entityDataWaypoints[i].Waypoints[1] : entityDataTargetLocation[i];
			}
			
			indexDataID++;
		}
	}
};