#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "Generics/Systems/FluxPrimeSpatialGridSystem.h"
#include "NavigationSystem.h"
#include "Detour/DetourNavMeshQuery.h"
#include "DetourCrowd/DetourPathCorridor.h"
#include "NavMesh/RecastNavMesh.h"
#include "Generics/Data/FluxPrimeNavigationData.h"
#include "Generics/Systems/Navigation/FluxPrimeDetourOwner.h"
#include "FluxPrimeBaseNavigationSystem.generated.h"

/** Accumulates scheduling wait time and stops at the configured bound. */
inline float AccumulateNavigationWaitTime(const float currentWaitTime, const float deltaTime)
{
	return FMath::Min(currentWaitTime + deltaTime, FluxNavigationConfig::MaxWaitTime);
}

/**
 * Priority used to order navigation requests. The result is clamped, so no out-of-range
 * value ever reaches the sort key.
 */
inline float ComputeNavigationPriority(const float waitTime, const float baseScore, const float arrivalBoost)
{
	const float RawScore = baseScore + (waitTime * FluxNavigationConfig::WaitTimeScoreScale) + arrivalBoost;
	return FMath::Clamp(RawScore, 0.0f, FluxNavigationConfig::MaxPriorityScore);
}
/**
 * Bounds a Detour straight-path result by the destination waypoint capacity.
 * Detour's result is unbounded, so an oversized path is truncated rather than treated as an error.
 */
inline int32 ClampWaypointCount(const int32 resultCount, const int32 capacity)
{
	return FMath::Clamp(resultCount, 0, FMath::Max(capacity, 0));
}

/**
 * Search extent used to pull an off-mesh crowd agent back to the navmesh.
 *
 * The extent is derived from the entity radius, which the navmesh already accounts for
 * through agent-radius erosion. Navigation owns this correction, so movement stays a pure
 * kinematic system and the constraint is applied at the start of the next navigation tick.
 */
inline FVector ComputeNavigationProjectionExtent(const int16 entitySize, const float sizeMultiplier)
{
	const float SafeSize = FMath::Max(1.0f, static_cast<float>(entitySize));
	const float SafeMultiplier = FMath::Max(sizeMultiplier, 0.0f);
	return FVector(SafeSize * SafeMultiplier, SafeSize * SafeMultiplier, SafeSize * 2.0f * SafeMultiplier);
}

USTRUCT()
struct FFluxPrimeBaseNavigationSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()

	friend class FFluxPrimeDetourOwnershipTest;
	
protected:
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
	/** Sole owner of the Detour query and path corridor. */
	FFluxPrimeDetourOwner Detour;
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
	// Stored entity corridors/waypoints cannot survive a rebuild. Never rebind them.
	bool EnsureDetourMatchesNavMesh()
	{
		if (!IsValid(RecastNavMesh) || RecastNavMesh->HasAnyFlags(RF_ClassDefaultObject)
			|| !Detour.MatchesUnchangedMesh(RecastNavMesh->GetRecastMesh()))
		{
			Detour.Reset();
			IsAllDataValid = false;
			return false;
		}
		return true;
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
		FFluxPrimeCrowdsWaypoint& outPath, uint8& outCountPath, const int options = DT_STRAIGHTPATH_AREA_CROSSINGS)
	{
		dtNavMeshQuery* NavQuery = Detour.GetQuery();
		if (!NavQuery || GetPathCount() <= 0) return false;

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

		const int32 maxWaypoints = static_cast<int32>(FluxConfig::WaypointsCount);
		const int32 resultCount = result.size();
		const int32 copiedCount = ClampWaypointCount(resultCount, maxWaypoints);

		if (resultCount > copiedCount)
		{
			UE_LOG(LogFluxPrime, Verbose,
				TEXT("Straight path returned %d waypoints; truncating to the configured WaypointsCount %d."),
				resultCount, copiedCount);
		}

		outCountPath = static_cast<uint8>(copiedCount);

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
		return Detour.GetCorridor().movePosition(dtPos, Detour.GetQuery(), &Filter);
	}
	
	bool MoveTargetPosition(const FVector& newTarget)
	{
		dtReal dtTarget[3];
		UnrealToRecast(newTarget, dtTarget);

		Detour.GetCorridor().moveTargetPosition(dtTarget, Detour.GetQuery(), &Filter);
		return true;
	}
	
	bool OptimizePathTopology()
	{
		return Detour.GetCorridor().optimizePathTopology(Detour.GetQuery(), &Filter);
	}
	
	void Reset(NavNodeRef startPoly, const FVector& startLocation)
	{
		dtReal dtStart[3];
		UnrealToRecast(startLocation, dtStart);
		Detour.GetCorridor().reset(startPoly, dtStart);
	}
	
	void SetCorridor(const FVector& target, const TArray<NavNodeRef>& corridorPath)
	{
		dtReal dtTarget[3];
		UnrealToRecast(target, dtTarget);
		Detour.GetCorridor().setCorridor(dtTarget, corridorPath.GetData(), corridorPath.Num());
	}
	
	const dtPolyRef* GetPath() const
	{
		return Detour.GetCorridor().getPath();
	}

	int32 GetPathCount() const
	{
		return Detour.GetCorridor().getPathCount();
	}
	
	void Reset(const FFluxPrimeCrowdsCorridor& corridor, const FVector& currentLocation)
	{
		dtReal dtStart[3];
		UnrealToRecast(currentLocation, dtStart);
		NavNodeRef startPoly = corridor.Corridors[0];
		Detour.GetCorridor().reset(startPoly, dtStart);
	}
	
	float SelectFloat(bool condition, float a, float b) const
	{
		return (condition)? a : b;
	}
	
	void Load(const FFluxPrimeCrowdsCorridor& corridor, uint8 corridorCount, const FVector& targetLocation)
	{
		dtReal dtTarget[3];
		UnrealToRecast(targetLocation, dtTarget);
		Detour.GetCorridor().setCorridor(dtTarget, corridor.Corridors.GetData(), corridorCount);
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
	
	void QueueFindPath(bool& needReplan, FVector& location, FVector& previousLocation, FVector& targetLocation, 
		FVector& lastReplanLocation, FVector& lastMoveLocation, int16& size, 
		FFluxPrimeCrowdsCorridor& outCorridor, uint8& outCorridorCount,
		FFluxPrimeCrowdsWaypoint& outWaypoints, uint8& outCountWaypoints)
	{
		size = FMath::Max(1, size);
		NavNodeRef startPoly, targetPoly;
		FVector3d startNearestPoint, targetNearestPoint;
		FVector searchExtent(size, size, size * 2.f);
		
		bool bStartFound = FindNearestPoly(location, searchExtent, startPoly, startNearestPoint);

		// A crowded agent can be pushed slightly off the mesh before the movement clamp runs.
		// Recover from its last known walkable start instead of failing the same query forever.
		if (!bStartFound && outCorridorCount > 0 &&
			FindNearestPoly(previousLocation, searchExtent, startPoly, startNearestPoint))
		{
			location = startNearestPoint;
			bStartFound = true;
		}

		if (!bStartFound)
		{
			outCorridorCount = 0;
			outCountWaypoints = 0;
			return;
		}

		if (!FindNearestPoly(targetLocation, searchExtent, targetPoly, targetNearestPoint))
		{
			outCorridorCount = 0;
			outCountWaypoints = 0;
			return;
		}
		
		if (!FindPath(startNearestPoint, targetNearestPoint, outCorridor, outCorridorCount))
		{
			outCorridorCount = 0;
			outCountWaypoints = 0;
			return;
		}
		
		lastReplanLocation = targetLocation;
		lastMoveLocation = lastReplanLocation;
		
		previousLocation = startNearestPoint;
		
		Reset(outCorridor, previousLocation);
		Load(outCorridor, outCorridorCount, lastReplanLocation);
		
		if (!FindStraightPath(startNearestPoint, targetNearestPoint, outWaypoints, outCountWaypoints))
		{
			// Do not keep the agent walking to a stale waypoint; ask for a fresh path next frame.
			outCountWaypoints = 0;
			needReplan = true;
			return;
		}
		
		Save(outCorridor, outCorridorCount);
		needReplan = false;
	}
	
	void QueueCorridor(bool& needReplan, FFluxPrimeCrowdsCorridor& corridors, uint8& countCorridor, FVector& location, FVector& previousLocation,
		FVector& targetLocation, FVector& lastMoveTargetLocation, FFluxPrimeCrowdsWaypoint& waypoints, uint8& countWaypoints, 
		int32& cellID, float& currentTime, float& lastOptimizeTime, float& lastMoveTargetTime)
	{
		int32 targetCellID = SpatialGridSystem.GetSpatialGridSystemsCellID(lastMoveTargetLocation);
		
		if (countCorridor == 0)
		{
			countWaypoints = 0;
			needReplan = true;
			return;
		}
		
		Reset(corridors, previousLocation);
		Load(corridors, countCorridor, lastMoveTargetLocation);
		
		if (!MovePosition(location))
		{
			needReplan = true;
			return;
		}
		
		if (currentTime - lastOptimizeTime > SelectFloat(cellID == targetCellID, OptimizeTimeSameCellID, OptimizeTimeDifferenceCellID))
		{
			OptimizePathTopology();
			lastOptimizeTime = currentTime;
		}
		
		if (currentTime - lastMoveTargetTime > SelectFloat(cellID == targetCellID, MoveTargetTimeSameCellID, MoveTargetTimeDifferenceCellID))
		{
			lastMoveTargetTime = currentTime;
			lastMoveTargetLocation = targetLocation;
			MoveTargetPosition(lastMoveTargetLocation);
		}
		
		if (!FindStraightPath(location, lastMoveTargetLocation, waypoints, countWaypoints))
		{
			// The corridor can survive while a straight path through it does not. Requeue a
			// path request instead of leaving the agent frozen on a zero-waypoint target.
			countWaypoints = 0;
			needReplan = true;
		}
		
		Save(corridors, countCorridor);
		previousLocation = location;
	}
};
