#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystem.h"
#include "FluxPrimeSpatialGridSystem.h"
#include "Cores/FluxPrimeStruct.h"
#include "NavigationSystem.h"
#include "Detour/DetourNavMeshQuery.h"
#include "DetourCrowd/DetourPathCorridor.h"
#include "NavMesh/RecastNavMesh.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeNavigationSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeNavigationSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool isDebug = false;
	
	TArray<int16>* crowdsSize = nullptr;
	TArray<int32>* crowdsCellID = nullptr;
	TArray<FVector>* crowdsCurrentLocation = nullptr;
	TArray<FVector>* crowdsPreviousLocation = nullptr;
	TArray<FFluxPrimeCrowdsCorridor>* crowdsCorridors = nullptr;
	TArray<FVector>* crowdsTarget = nullptr;
	TArray<FVector>* crowdsCurrentTarget = nullptr;
	TArray<FVector>* crowdsLastReplanTarget = nullptr;
	TArray<FVector>* crowdsLastMoveTarget = nullptr;
	TArray<float>* crowdsLastOptimizeTime = nullptr;
	TArray<float>* crowdsLastMoveTargetTime = nullptr;
	TArray<uint8>* crowdsCountCorridor = nullptr;
	TArray<bool>* crowdsNeedReplan = nullptr;
	TArray<FFluxPrimeCrowdsWaypoint>* crowdsWaypoints = nullptr;
	TArray<int8>* crowdsCountWaypoints = nullptr;
	
	UPROPERTY()
	uint16 queuePathCountPerFrame = 0;
	UPROPERTY()
	uint16 queueCorridorCountPerFrame = 0;
	
	UPROPERTY()
	float optimizeTimeSameCellID = 0.0f;
	UPROPERTY()
	float optimizeTimeDifferenceCellID = 0.0f;
	
	UPROPERTY()
	float moveTargetTimeSameCellID = 0.0f;
	UPROPERTY()
	float moveTargetTimeDifferenceCellID = 0.0f;
	
	UPROPERTY(EditAnywhere)
	FFluxPrimeSpatialGridSystemsContext contextSpatialGrid;
};

USTRUCT(BlueprintType)
struct FFluxPrimeNavigationSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
	UPROPERTY()
	bool IsDebug = false;
	
	uint16* MemberActive = nullptr;
	TArray<int16>* CrowdsSize = nullptr;
	TArray<int32>* CrowdsCellID = nullptr;
	TArray<FVector>* CrowdsCurrentLocation = nullptr;
	TArray<FVector>* CrowdsPreviousLocation = nullptr;
	TArray<FFluxPrimeCrowdsCorridor>* CrowdsCorridors = nullptr;
	TArray<FVector>* CrowdsTarget = nullptr;
	TArray<FVector>* CrowdsCurrentTarget = nullptr;
	TArray<FVector>* CrowdsLastReplanTarget = nullptr;
	TArray<FVector>* CrowdsLastMoveTarget = nullptr;
	TArray<float>* CrowdsLastOptimizeTime = nullptr;
	TArray<float>* CrowdsLastMoveTargetTime = nullptr;
	TArray<uint8>* CrowdsCountCorridor = nullptr;
	TArray<bool>* CrowdsNeedReplan = nullptr;
	TArray<FFluxPrimeCrowdsWaypoint>* CrowdsWaypoints = nullptr;
	TArray<int8>* CrowdsCountWaypoints = nullptr;
	
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
	
	UPROPERTY()
	uint16 QueuePathCountPerFrame = 20;
	UPROPERTY()
	uint16 QueueCorridorCountPerFrame = 20;
	
	UPROPERTY()
	float OptimizeTimeSameCellID = 0.15f;
	UPROPERTY()
	float OptimizeTimeDifferenceCellID = 0.5f;
	
	UPROPERTY()
	float MoveTargetTimeSameCellID = 0.25f;
	UPROPERTY()
	float MoveTargetTimeDifferenceCellID = 1.5f;
	
	FFluxPrimeSpatialGridSystem SpatialGridSystems;
	
private:
	void ShowDebug(FVector path, FColor color)
	{
		DrawDebugSphere(
				World,
				path,
				50.0f,
				8,
				color,
				false,
				.1f,
				0,
				3.0f
			);
	}
	
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
		FFluxPrimeCrowdsWaypoint& outPath, int8& outCountPath, const int options = DT_STRAIGHTPATH_AREA_CROSSINGS)
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
	
	void Reset(const FFluxPrimeCrowdsCorridor& crowdCorridor, const FVector& currentLocation)
	{
		dtReal dtStart[3];
		UnrealToRecast(currentLocation, dtStart);
		NavNodeRef startPoly = crowdCorridor.Corridors[0];
		Corridor.reset(startPoly, dtStart);
	}
	
	float SelectFloat(bool condition, float a, float b) const
	{
		return (condition)? a : b;
	}
	
	void Load(const FFluxPrimeCrowdsCorridor& crowdCorridor, uint8 corridorCount, const FVector& targetLocation)
	{
		dtReal dtTarget[3];
		UnrealToRecast(targetLocation, dtTarget);
		Corridor.setCorridor(dtTarget, crowdCorridor.Corridors.GetData(), corridorCount);
	}
	
	void Save(FFluxPrimeCrowdsCorridor& crowdCorridor, uint8& corridorCount)
	{
		corridorCount = FMath::Min(GetPathCount(), FluxConfig::CorridorsCount);

		const dtPolyRef* path = GetPath();

		for (int32 i = 0; i < corridorCount; ++i)
		{
			crowdCorridor.Corridors[i] = path[i];
		}
	}
	
	void CrowdsQueueFindPath(uint16& queuePath, bool& needReplan, FVector& crowdLocation, FVector& currentTargetLocation, 
		FVector& lastReplanTargetLocation, FVector& lastMoveTargetLocation, int16& crowdSize, FFluxPrimeCrowdsCorridor& outCrowdCorridor, uint8& outCrowdCorridorCount)
	{	
		if (queuePath > QueuePathCountPerFrame) return;

		NavNodeRef startPoly, targetPoly;
		FVector3d startNearestPoint, targetNearestPoint;
		FVector searchExtent( crowdSize, crowdSize, crowdSize * 2.f);
		
		bool bFindNearestPoly = FindNearestPoly(crowdLocation, searchExtent, startPoly, startNearestPoint) 
			&& FindNearestPoly(currentTargetLocation, searchExtent, targetPoly, targetNearestPoint);
		
		if (!bFindNearestPoly) return;
		
		if (!FindPath(startNearestPoint, targetNearestPoint, outCrowdCorridor, outCrowdCorridorCount))
		{
			outCrowdCorridorCount = 0;
			return;
		}
		
		lastReplanTargetLocation = currentTargetLocation;
		lastMoveTargetLocation = lastReplanTargetLocation;
		
		Load(outCrowdCorridor, outCrowdCorridorCount, lastReplanTargetLocation);
		Save(outCrowdCorridor, outCrowdCorridorCount);
		
		needReplan = false;
		queuePath++;

		ShowDebug(startNearestPoint, FColor::Red);
		ShowDebug(targetNearestPoint, FColor::Red);
	}
	
	void CrowdsQueueCorridor(uint16& queueCorridor, FFluxPrimeCrowdsCorridor& crowdCorridors, uint8& crowdCountCorridor, FVector& crowdLocation, FVector& crowdPreviousLocation,
		FVector& crowdTarget, FVector& crowdLastMoveTarget, FFluxPrimeCrowdsWaypoint& crowdWaypoints, int8& crowdCountWaypoints, 
		int32& cellID, float& currentTime, float& lastOptimizeTime, float& lastMoveTargetTime, bool& needMoveTarget)
	{
		// crowdLastReplanTarget sebelumnya lokasi dari last replan target
		
		if (queueCorridor > QueueCorridorCountPerFrame) return;
		
		int32 targetCellID = SpatialGridSystems.GetSpatialGridSystemsCellID(crowdLastMoveTarget);
		
		if (crowdCountCorridor == 0)
		{
			crowdCountWaypoints = 0;
			return;
		}
		
		Reset(crowdCorridors, crowdPreviousLocation);
		Load(crowdCorridors, crowdCountCorridor, crowdLastMoveTarget);
		
		if (!MovePosition(crowdLocation)) return;
		
		if (currentTime - lastOptimizeTime > SelectFloat(cellID == targetCellID, OptimizeTimeSameCellID, OptimizeTimeDifferenceCellID))
		{
			if (!OptimizePathTopology()) return;
			lastOptimizeTime = currentTime;
		}
		
		if (needMoveTarget || currentTime - lastMoveTargetTime > SelectFloat(cellID == targetCellID, MoveTargetTimeSameCellID, MoveTargetTimeDifferenceCellID))
		{
			needMoveTarget = !MoveTargetPosition(crowdTarget);
			if (needMoveTarget) return;
			lastMoveTargetTime = currentTime;
			crowdLastMoveTarget = crowdTarget;
		}
		
		if (!FindStraightPath(crowdLocation, crowdLastMoveTarget, 
			crowdWaypoints,crowdCountWaypoints)) crowdCountWaypoints = 0;
		
		Save(crowdCorridors, crowdCountCorridor);
		
		crowdPreviousLocation = crowdLocation;
		
		queueCorridor++;
	}
	
public:
	void InitializedNavigationSystems(FFluxPrimeNavigationSystemsContext context)
	{
		check(context.contextSpatialGrid.memberActive);
		check(context.crowdsCellID);
		check(context.crowdsCurrentLocation);
		check(context.crowdsPreviousLocation);
		check(context.crowdsTarget);
		check(context.crowdsSize);
		check(context.crowdsCorridors);
		check(context.crowdsCurrentTarget);
		check(context.crowdsLastReplanTarget);
		check(context.crowdsLastOptimizeTime);
		check(context.crowdsLastMoveTargetTime);
		check(context.crowdsLastMoveTarget);
		check(context.crowdsCountCorridor);
		check(context.crowdsNeedReplan);
		check(context.crowdsWaypoints);
		check(context.crowdsCountWaypoints);

		World = context.contextSpatialGrid.world;
		IsDebug = context.isDebug;
		MemberActive = context.contextSpatialGrid.memberActive;

		CrowdsCurrentLocation = context.crowdsCurrentLocation;
		CrowdsPreviousLocation = context.crowdsPreviousLocation;
		CrowdsCellID = context.crowdsCellID;
		CrowdsSize = context.crowdsSize;
		CrowdsCorridors = context.crowdsCorridors;
		CrowdsTarget = context.crowdsTarget;
		CrowdsCurrentTarget = context.crowdsCurrentTarget;
		CrowdsLastReplanTarget = context.crowdsLastReplanTarget;
		CrowdsLastOptimizeTime = context.crowdsLastOptimizeTime;
		CrowdsLastMoveTargetTime = context.crowdsLastMoveTargetTime;
		CrowdsLastMoveTarget = context.crowdsLastMoveTarget;
		CrowdsCountCorridor = context.crowdsCountCorridor;
		CrowdsNeedReplan = context.crowdsNeedReplan;
		CrowdsWaypoints = context.crowdsWaypoints;
		CrowdsCountWaypoints = context.crowdsCountWaypoints;
		
		QueuePathCountPerFrame = context.queuePathCountPerFrame;
		QueueCorridorCountPerFrame = context.queueCorridorCountPerFrame;
		OptimizeTimeSameCellID = context.optimizeTimeSameCellID;
		OptimizeTimeDifferenceCellID = context.optimizeTimeDifferenceCellID;
		MoveTargetTimeSameCellID = context.moveTargetTimeSameCellID;
		MoveTargetTimeDifferenceCellID = context.moveTargetTimeDifferenceCellID;
		
		SpatialGridSystems.InitializedSpatialGridSystems(context.contextSpatialGrid);
		SpatialGridSystems.BakeSpatialGridSystems();
		
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
	}
	
	void UpdateNavigationSystems()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_Systems);
		
		SpatialGridSystems.UpdateSpatialGridSystem();
		
		float CurrentTime = World->GetTimeSeconds();
		
		auto& crowdsCurrentLocation = *CrowdsCurrentLocation;
		auto& crowdsPreviousLocation = *CrowdsPreviousLocation;
		auto& crowdsSize = *CrowdsSize;
		auto& crowdsCellId = *CrowdsCellID;
		auto& crowdsCorridors = *CrowdsCorridors;
		auto& crowdsTarget = *CrowdsTarget;
		auto& crowdsCurrentTarget = *CrowdsCurrentTarget;
		auto& crowdsLastReplanTarget = *CrowdsLastReplanTarget;
		auto& crowdsLastMoveTarget = *CrowdsLastMoveTarget;
		auto& crowdsLastOptimizeTime = *CrowdsLastOptimizeTime;
		auto& crowdsLastMoveTargetTime = *CrowdsLastMoveTargetTime;
		auto& crowdsCountCorridor = *CrowdsCountCorridor;
		auto& crowdsNeedReplan = *CrowdsNeedReplan;
		auto& crowdsWaypoints = *CrowdsWaypoints;
		auto& crowdsCountWaypoints = *CrowdsCountWaypoints;
		
		uint16 queuePath = 0;
		uint16 queueCorridor = 0;
		bool crowdsNeedMoveTarget = false;
		for (int i = 0; i < *MemberActive; ++i)
		{
			crowdsNeedMoveTarget = false;
			
			const float distanceReplan = FVector::DistSquaredXY(crowdsLastReplanTarget[i], crowdsTarget[i]);
			const float distanceMoveTarget = FVector::DistSquaredXY(crowdsLastMoveTarget[i], crowdsTarget[i]);
			
			crowdsNeedReplan[i] = distanceReplan > 1500 || crowdsNeedReplan[i];
			crowdsNeedMoveTarget = distanceMoveTarget > 400;
			
			if (crowdsNeedReplan[i])
			{
				CrowdsQueueFindPath(queuePath, crowdsNeedReplan[i], crowdsCurrentLocation[i], crowdsTarget[i], 
					crowdsLastReplanTarget[i], crowdsLastMoveTarget[i], crowdsSize[i],crowdsCorridors[i], crowdsCountCorridor[i]);
			}
			else
			{
				CrowdsQueueCorridor(queueCorridor, crowdsCorridors[i], crowdsCountCorridor[i], crowdsCurrentLocation[i], crowdsPreviousLocation[i],
					crowdsTarget[i], crowdsLastMoveTarget[i], crowdsWaypoints[i], crowdsCountWaypoints[i],
				crowdsCellId[i], CurrentTime, crowdsLastOptimizeTime[i], crowdsLastMoveTargetTime[i], crowdsNeedMoveTarget);
			}
			
			crowdsCurrentTarget[i] = (crowdsCountWaypoints[i] > 1)? crowdsWaypoints[i].Waypoints[1] : crowdsTarget[i];
		}
	}
	
	void EndPlayNavigationSystems()
	{
		World = nullptr;
	}
};
