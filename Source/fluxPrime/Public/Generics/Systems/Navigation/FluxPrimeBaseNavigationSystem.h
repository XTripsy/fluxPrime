#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "Generics/Systems/FluxPrimeSpatialGridSystem.h"
#include "NavigationSystem.h"
#include "Detour/DetourNavMeshQuery.h"
#include "DetourCrowd/DetourPathCorridor.h"
#include "NavMesh/RecastNavMesh.h"
#include "Generics/Data/FluxPrimeNavigationData.h"
#include "FluxPrimeBaseNavigationSystem.generated.h"

USTRUCT()
struct FFluxPrimeBaseNavigationSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
protected:
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
	FFluxPrimeSpatialGridSystem SpatialGridSystem = FFluxPrimeSpatialGridSystem();
	
protected:
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
		
		bool bFindNearestPoly = FindNearestPoly(location, searchExtent, startPoly, startNearestPoint) 
			&& FindNearestPoly(targetLocation, searchExtent, targetPoly, targetNearestPoint);
		
		if (!bFindNearestPoly) return;
		
		if (!FindPath(startNearestPoint, targetNearestPoint, outCorridor, outCorridorCount))
		{
			outCorridorCount = 0;
			return;
		}
		
		lastReplanLocation = targetLocation;
		lastMoveLocation = lastReplanLocation;
		
		Load(outCorridor, outCorridorCount, lastReplanLocation);
		Save(outCorridor, outCorridorCount);
		
		needReplan = false;
	}
	
	void QueueCorridor(FFluxPrimeCrowdsCorridor& corridors, uint8& countCorridor, FVector& location, FVector& previousLocation,
		FVector& targetLocation, FVector& lastMoveTargetLocation, FFluxPrimeCrowdsWaypoint& waypoints, uint8& countWaypoints, 
		int32& cellID, float& currentTime, float& lastOptimizeTime, float& lastMoveTargetTime)
	{
		int32 targetCellID = SpatialGridSystem.GetSpatialGridSystemsCellID(lastMoveTargetLocation);
		
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
};
