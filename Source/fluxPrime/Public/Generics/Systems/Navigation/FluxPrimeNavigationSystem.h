#pragma once

#include "CoreMinimal.h"
#include "Algo/Sort.h"
#include "fluxPrime.h"
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

/**
 * Scheduling entry for one navigation request.
 *
 * The full task payload (corridors + waypoints) is only materialised for the entries that
 * survive the per-frame queue limit. Building and sorting the queue therefore no longer
 * copies the payload for every active entity.
 */
struct FFluxPrimeNavigationQueueEntry
{
	float PriorityScore = 0.0f;
	int16 DataStoreIndex = 0;
	int32 InternalIndex = 0;

	/** Ascending sort puts the highest priority score first. */
	bool operator<(const FFluxPrimeNavigationQueueEntry& Other) const
	{
		return PriorityScore > Other.PriorityScore;
	}
};

/**
 * Keeps the `limit` highest-priority entries at the front of the queue and returns how many
 * are ready to process.
 *
 * Only that many entries are ever consumed per frame, so a full sort of the queue pays for an
 * ordering nobody reads. Selection runs as a bounded min-heap (O(n log limit)) and only the
 * winners are sorted afterwards. Entries past the returned count are rejected ones, in no
 * particular order, and nothing behind the selection outranks it.
 */
inline int32 SelectHighestPriorityEntries(FFluxPrimeNavigationQueueEntry* entries, const int32 count, const int32 limit)
{
	if (count <= 0) return 0;

	const int32 selected = FMath::Min(limit, count);
	if (selected <= 0) return 0;

	if (selected == count)
	{
		Algo::Sort(TArrayView<FFluxPrimeNavigationQueueEntry>(entries, count));
		return selected;
	}

	// Min-heap on the score, so the root is the weakest entry currently kept and a candidate
	// has to beat a single comparison before it replaces it.
	auto SiftDown = [entries, selected](int32 root)
	{
		for (;;)
		{
			const int32 left = 2 * root + 1;
			if (left >= selected) return;

			const int32 right = left + 1;
			const int32 weakest =
				(right < selected && entries[right].PriorityScore < entries[left].PriorityScore) ? right : left;

			if (entries[root].PriorityScore <= entries[weakest].PriorityScore) return;

			Swap(entries[root], entries[weakest]);
			root = weakest;
		}
	};

	for (int32 i = selected / 2 - 1; i >= 0; --i)
	{
		SiftDown(i);
	}

	for (int32 i = selected; i < count; ++i)
	{
		if (entries[i].PriorityScore > entries[0].PriorityScore)
		{
			// Swap rather than overwrite so the rejected entry stays behind the selection: the
			// first `selected` entries end up as the winners and nothing after them outranks them.
			Swap(entries[0], entries[i]);
			SiftDown(0);
		}
	}

	Algo::Sort(TArrayView<FFluxPrimeNavigationQueueEntry>(entries, selected));

	return selected;
}

USTRUCT(BlueprintType)
struct FFluxPrimeNavigationSystem : public FFluxPrimeBaseNavigationSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FNavigationSystemDataTagConfig DataTagConfig;

	UPROPERTY(EditAnywhere, Category = "Navigation", meta = (ClampMin = "0", Units = "cm"))
	float ArrivalRadius = 60.0f;
	UPROPERTY(EditAnywhere, Category = "Navigation", meta = (ClampMin = "0"))
	float ArrivalSizeMultiplier = 1.5f;
	UPROPERTY(EditAnywhere, Category = "Navigation")
	bool bConstrainToNavMesh = true;
	UPROPERTY(EditAnywhere, Category = "Navigation", meta = (ClampMin = "0"))
	float NavMeshProjectionSizeMultiplier = 1.5f;

	/**
	 * How far an agent may drift from its last verified projection before the navmesh query
	 * runs again, in cm. Zero keeps verifying every frame that the agent moved at all.
	 *
	 * The gate bounds the error to this distance; it never accumulates, because the budget is
	 * only refreshed by a real projection.
	 */
	UPROPERTY(EditAnywhere, Category = "Navigation", meta = (ClampMin = "0", Units = "cm"))
	float ConstrainMovementThreshold = 10.0f;

	TArray<int16> EntityDataCurrentLocationID, EntityDataPreviousLocationID, EntityDataSizeID, EntityDataCellIdID, EntityDataCorridorsID, EntityDataTargetLocationID, EntityDataCurrentWaypointLocationID, EntityDataLastReplanLocationID, EntityDataLastMoveLocationID, EntityDataWaitTimeID, EntityDataLastOptimizeTimeID, EntityDataLastMoveTargetTimeID, EntityDataCountCorridorID, EntityDataNeedReplanID, EntityDataWaypointsID, EntityDataCountWaypointsID;
	TArray<FFluxPrimeNavigationQueueEntry> EntityPathQueue;
	TArray<FFluxPrimeNavigationQueueEntry> EntityCorridorQueue;

	/** Last poly each entity projected onto; only a hint for the constrain fast path. */
	TArray<NavNodeRef> ConstrainPolyCache;

	/** Position each cached poly was verified to hold; the gate compares against it. */
	TArray<FVector> ConstrainPositionCache;
	
public:
	void Initialized(FFluxPrimeNavigationSystemsContext context)
	{
		check(context.world);

		Detour.Reset();
		World = context.world;
		
		SpatialGridSystem.Initialized(*context.dataStores);
		
		NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
		check(NavSys);

		NavData = NavSys->GetDefaultNavDataInstance();
		check(NavData);

		RecastNavMesh = Cast<ARecastNavMesh>(NavData);
		if (!IsValid(RecastNavMesh) || RecastNavMesh->HasAnyFlags(RF_ClassDefaultObject))
		{
			UE_LOG(LogFluxPrime, Error, TEXT("Navigation system cannot initialize: the default navigation data is not a Recast navmesh."));
			IsAllDataValid = false;
			return;
		}
		
		dtNavMesh* DtNavMesh = RecastNavMesh->GetRecastMesh();
		if (!DtNavMesh)
		{
			UE_LOG(LogFluxPrime, Error, TEXT("Navigation system cannot initialize: the navmesh has no Detour mesh."));
			IsAllDataValid = false;
			return;
		}

		// The same filter the engine's own queries use, so projections and path queries agree.
		NavFilter = NavData->GetDefaultQueryFilter();

		// Same A* node budget the replaced FindPathSync query handed to its query. The pool is
		// allocated once here, so this sizing has to cover the largest search it will ever run.
		const uint32 FilterMaxSearchNodes = NavFilter ? NavFilter->GetMaxSearchNodes() : 0;
		const int32 MaxSearchNodes = FMath::Max(2048,
			static_cast<int32>(FMath::Min<uint32>(FilterMaxSearchNodes, static_cast<uint32>(TNumericLimits<int32>::Max()))));

		if (!Detour.Initialize(DtNavMesh, MaxSearchNodes, 256))
		{
			UE_LOG(LogFluxPrime, Error,
				TEXT("Navigation system cannot initialize: Detour query or corridor allocation failed."));
			IsAllDataValid = false;
			return;
		}

		// FPathFindingQuery defaults to requiring a navigable end location.
		Detour.GetQuery()->setRequireNavigableEndLocation(true);
		
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
		
		EntityPathQueue.Init(FFluxPrimeNavigationQueueEntry(), dataTotalCount);
		EntityCorridorQueue.Init(FFluxPrimeNavigationQueueEntry(), dataTotalCount);
		ConstrainPolyCache.Init(INVALID_NAVNODEREF, dataTotalCount);
		ConstrainPositionCache.Init(FVector::ZeroVector, dataTotalCount);

	}
	
	void SyncNewTarget(FFluxPrimeArchetypeDataStore& dataStore, int32 indexDataID, int32 indexData, bool newRequestNeedReplan)
	{
		reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataNeedReplanID[indexDataID]))[indexData].ValueBool = newRequestNeedReplan;
	}

	/**
	 * Pulls every agent back onto the navmesh before path planning and grid updates.
	 *
	 * Movement runs after navigation, so this is the next-tick constraint: boids can still
	 * push an agent slightly off the mesh for the current frame, and navigation recovers it
	 * before using its position again. The navmesh is already eroded by the agent radius,
	 * so a normal movement step cannot reach the far side of a wall between two ticks.
	 *
	 * The per-agent navmesh query is the expensive part, so it is gated twice: an agent that
	 * has not moved past ConstrainMovementThreshold since its last verified projection skips
	 * the query entirely, and one that has pays only a closestPointOnPoly check of the cached
	 * poly instead of a full projection.
	 */
	void ConstrainLocationsToNavMesh(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		if (!bConstrainToNavMesh || !IsValid(RecastNavMesh)) return;

		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_Constrain);

		const float ThresholdSquared = FMath::Square(FMath::Max(ConstrainMovementThreshold, 0.0f));

		int32 indexDataID = 0;
		int32 cacheOffset = 0;
		for (auto& dataStore : dataStores)
		{
			FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentLocationID[indexDataID]));
			FFluxPrimeWrapperInt16* entityDataSize = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataSizeID[indexDataID]));
			indexDataID++;

			const int32 entityCount = dataStore.GetTotalEntityCount();
			const bool bCacheFits = ConstrainPolyCache.Num() >= cacheOffset + entityCount
				&& ConstrainPositionCache.Num() >= cacheOffset + entityCount;
			NavNodeRef* polyCache = bCacheFits ? ConstrainPolyCache.GetData() + cacheOffset : nullptr;
			FVector* positionCache = bCacheFits ? ConstrainPositionCache.GetData() + cacheOffset : nullptr;
			cacheOffset += entityCount;

			if (!entityDataLocation || !entityDataSize) continue;

			const int32 activeCount = dataStore.GetActiveEntityCount();
			for (int32 i = 0; i < activeCount; ++i)
			{
				const FVector Extent = ComputeNavigationProjectionExtent(entityDataSize[i].ValueInt16, NavMeshProjectionSizeMultiplier);

				if (polyCache)
				{
					const bool bWithinBudget = polyCache[i] != INVALID_NAVNODEREF
						&& IsWithinConstrainThreshold(positionCache[i], entityDataLocation[i], ThresholdSquared);

					// Only an agent that spent its drift budget pays for the poly check, and a
					// passing check re-anchors the budget instead of leaving the gate shut.
					if (bWithinBudget && IsProjectionIdentity(polyCache[i], entityDataLocation[i], Extent))
					{
						positionCache[i] = entityDataLocation[i];
						continue;
					}
				}

				TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_ConstrainProject);

				FNavLocation ProjectedLocation;
				if (!RecastNavMesh->ProjectPoint(entityDataLocation[i], ProjectedLocation, Extent, NavFilter)) continue;

				entityDataLocation[i].X = ProjectedLocation.Location.X;
				entityDataLocation[i].Y = ProjectedLocation.Location.Y;
				if (polyCache)
				{
					polyCache[i] = ProjectedLocation.NodeRef;
					positionCache[i] = entityDataLocation[i];
				}
			}
		}
	}

	/** Copies one row into the payload the queue processors expect. */
	void LoadNavigationTask(const FFluxPrimeArchetypeDataStore& dataStore, int32 indexDataID, int32 indexInternalData,
		FFluxPrimeNavigationTask& outTask) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_LoadTask);

		outTask.EntityDataSize = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataSizeID[indexDataID]))[indexInternalData].ValueInt16;
		outTask.EntityDataCellId = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataCellIdID[indexDataID]))[indexInternalData].ValueInt32;
		outTask.EntityDataCurrentTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointLocationID[indexDataID]))[indexInternalData];
		outTask.EntityDataCurrentLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentLocationID[indexDataID]))[indexInternalData];
		outTask.EntityDataPreviousLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataPreviousLocationID[indexDataID]))[indexInternalData];
		outTask.EntityDataTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]))[indexInternalData];
		outTask.EntityDataLastReplanLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastReplanLocationID[indexDataID]))[indexInternalData];
		outTask.EntityDataLastMoveLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastMoveLocationID[indexDataID]))[indexInternalData];
		outTask.EntityDataWaitTime = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataWaitTimeID[indexDataID]))[indexInternalData].ValueFloat;
		outTask.EntityDataLastOptimizeTime = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataLastOptimizeTimeID[indexDataID]))[indexInternalData].ValueFloat;
		outTask.EntityDataLastMoveTargetTime = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataLastMoveTargetTimeID[indexDataID]))[indexInternalData].ValueFloat;
		outTask.EntityDataCorridors = reinterpret_cast<FFluxPrimeNavigationCorridor*>(dataStore.GetRawBufferData(EntityDataCorridorsID[indexDataID]))[indexInternalData];
		outTask.EntityDataCountCorridor = reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountCorridorID[indexDataID]))[indexInternalData].ValueUint8;
		outTask.EntityDataNeedReplan = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataNeedReplanID[indexDataID]))[indexInternalData].ValueBool;
		outTask.EntityDataWaypoints = reinterpret_cast<FFluxPrimeNavigationWaypoint*>(dataStore.GetRawBufferData(EntityDataWaypointsID[indexDataID]))[indexInternalData];
		outTask.EntityDataCountWaypoints = reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountWaypointsID[indexDataID]))[indexInternalData].ValueUint8;
	}

	/**
	 * Records a queue entry without copying the task payload.
	 *
	 * Only QueuePathCountPerFrame / QueueCorridorCountPerFrame entries are ever processed,
	 * so the corridor and waypoint payload is materialised after the sort, not before it.
	 */
	static void EnqueueNavigationRequest(TArray<FFluxPrimeNavigationQueueEntry>& queue, int32& count,
		int32 dataStoreIndex, int32 internalIndex, float priorityScore)
	{
		if (!queue.IsValidIndex(count)) return;

		queue[count].PriorityScore = priorityScore;
		queue[count].DataStoreIndex = static_cast<int16>(dataStoreIndex);
		queue[count].InternalIndex = internalIndex;
		count++;
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_Systems);

		if (!IsAllDataValid)
		{
			UE_LOG(LogFluxPrime, Verbose, TEXT("NAVIGATION SYSTEM :: data invalid; system skipped"));
			return;
		}

		if (!EnsureDetourMatchesNavMesh())
		{
			UE_LOG(LogFluxPrime, Error, TEXT("NAVIGATION SYSTEM :: navmesh unavailable or changed. Runtime rebuilds are unsupported; navigation disabled. Restart with fresh entity corridors and waypoints."));
			return;
		}
		
		ConstrainLocationsToNavMesh(dataStores);
		SpatialGridSystem.UpdateCellIds(dataStores);
		
		float currentTime = World->GetTimeSeconds();
		
		int32 indexDataID = 0;
		int32 countPathQueue = 0, countCorridorQueue = 0;
		int32 lastReplanTargetCellId = 0, targetCellId = 0;

		{
			TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_QueueBuild);

			for (auto& dataStore : dataStores)
			{
				FFluxPrimeWrapperInt16* entityDataSize = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataSizeID[indexDataID]));
				FFluxPrimeWrapperInt32* entityDataCellId = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataCellIdID[indexDataID]));
				FVector* entityDataCurrentTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointLocationID[indexDataID]));
				FVector* entityDataCurrentLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentLocationID[indexDataID]));
				FVector* entityDataTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]));
				FVector* entityDataLastReplanLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastReplanLocationID[indexDataID]));
				FFluxPrimeWrapperFloat* entityDataWaitTime = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataWaitTimeID[indexDataID]));
				FFluxPrimeWrapperBool* entityDataNeedReplan = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataNeedReplanID[indexDataID]));
				FFluxPrimeNavigationWaypoint* entityDataWaypoints = reinterpret_cast<FFluxPrimeNavigationWaypoint*>(dataStore.GetRawBufferData(EntityDataWaypointsID[indexDataID]));
				FFluxPrimeWrapperUint8* entityDataCountWaypoints = reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountWaypointsID[indexDataID]));

				check(entityDataSize && entityDataCellId && entityDataCurrentTargetLocation && entityDataCurrentLocation);
				check(entityDataTargetLocation && entityDataLastReplanLocation && entityDataWaitTime && entityDataNeedReplan);
				check(entityDataWaypoints && entityDataCountWaypoints);

				for (int i = 0; i < dataStore.GetActiveEntityCount(); ++i)
				{
					targetCellId = SpatialGridSystem.GetSpatialGridSystemsCellID(entityDataTargetLocation[i]);
					lastReplanTargetCellId = SpatialGridSystem.GetSpatialGridSystemsCellID(entityDataLastReplanLocation[i]);

					entityDataNeedReplan[i].ValueBool = (targetCellId != lastReplanTargetCellId) || entityDataNeedReplan[i].ValueBool;

					entityDataWaitTime[i].ValueFloat = AccumulateNavigationWaitTime(entityDataWaitTime[i].ValueFloat, deltaTime);

					const float arrivalRadiusSq = FMath::Square(FMath::Max(ArrivalRadius, entityDataSize[i].ValueInt16 * ArrivalSizeMultiplier));
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

					const float baseScore = (targetCellId == entityDataCellId[i].ValueInt32) ? 5.0f : 0.0f;

					if (entityDataNeedReplan[i].ValueBool)
					{
						if (entityDataCountWaypoints[i].ValueUint8 <= 1)
							entityDataCurrentTargetLocation[i] = entityDataCurrentLocation[i];

						EnqueueNavigationRequest(EntityPathQueue, countPathQueue, indexDataID, i,
							ComputeNavigationPriority(entityDataWaitTime[i].ValueFloat, baseScore, 0.0f));
					}
					else
					{
						const float arrivalBoost = bReachedCurrentWaypoint ? 50.0f : 0.0f;

						EnqueueNavigationRequest(EntityCorridorQueue, countCorridorQueue, indexDataID, i,
							ComputeNavigationPriority(entityDataWaitTime[i].ValueFloat, baseScore, arrivalBoost));
					}
				}

				indexDataID++;
			}
		}
		
		int32 indexInternalData = 0;
		
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_QueueProcess);

		int32 queuePathLimit = 0;
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_PathSelect);
			queuePathLimit = SelectHighestPriorityEntries(EntityPathQueue.GetData(), countPathQueue, QueuePathCountPerFrame);
		}

		for (int i = 0; i < queuePathLimit; ++i)
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_PathEntry);
			const FFluxPrimeNavigationQueueEntry& data = EntityPathQueue[i];

			indexDataID = data.DataStoreIndex;
			indexInternalData = data.InternalIndex;
			FFluxPrimeArchetypeDataStore& dataStore = dataStores[indexDataID];

			FFluxPrimeNavigationTask task;
			LoadNavigationTask(dataStore, indexDataID, indexInternalData, task);
			
			QueueFindPath(task.EntityDataNeedReplan, task.EntityDataCurrentLocation, task.EntityDataPreviousLocation,
						task.EntityDataTargetLocation, task.EntityDataLastReplanLocation, task.EntityDataLastMoveLocation, 
						task.EntityDataSize, task.EntityDataCorridors, task.EntityDataCountCorridor,
						task.EntityDataWaypoints, task.EntityDataCountWaypoints);
        
			reinterpret_cast<FFluxPrimeNavigationCorridor*>(dataStore.GetRawBufferData(EntityDataCorridorsID[indexDataID]))[indexInternalData] = task.EntityDataCorridors;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataPreviousLocationID[indexDataID]))[indexInternalData] = task.EntityDataPreviousLocation;
			// QueueFindPath may snap an off-mesh agent back to its last walkable position.
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentLocationID[indexDataID]))[indexInternalData] = task.EntityDataCurrentLocation;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]))[indexInternalData] = task.EntityDataTargetLocation;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastReplanLocationID[indexDataID]))[indexInternalData] = task.EntityDataLastReplanLocation;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastMoveLocationID[indexDataID]))[indexInternalData] = task.EntityDataLastMoveLocation;
			reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountCorridorID[indexDataID]))[indexInternalData].ValueUint8 = task.EntityDataCountCorridor;
			reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataNeedReplanID[indexDataID]))[indexInternalData].ValueBool = task.EntityDataNeedReplan;
			reinterpret_cast<FFluxPrimeNavigationWaypoint*>(dataStore.GetRawBufferData(EntityDataWaypointsID[indexDataID]))[indexInternalData] = task.EntityDataWaypoints;
			reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountWaypointsID[indexDataID]))[indexInternalData].ValueUint8 = task.EntityDataCountWaypoints;
			reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataWaitTimeID[indexDataID]))[indexInternalData].ValueFloat = 0.0f;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointLocationID[indexDataID]))[indexInternalData] = (task.EntityDataCountWaypoints > 1) ? task.EntityDataWaypoints.Waypoints[1] : task.EntityDataCurrentLocation;
		}
		
		int32 queueCorridorLimit = 0;
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_CorridorSelect);
			queueCorridorLimit = SelectHighestPriorityEntries(EntityCorridorQueue.GetData(), countCorridorQueue, QueueCorridorCountPerFrame);
		}

		for (int i = 0; i < queueCorridorLimit; ++i)
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_CorridorEntry);
			const FFluxPrimeNavigationQueueEntry& data = EntityCorridorQueue[i];

			indexDataID = data.DataStoreIndex;
			indexInternalData = data.InternalIndex;
			FFluxPrimeArchetypeDataStore& dataStore = dataStores[indexDataID];

			FFluxPrimeNavigationTask task;
			LoadNavigationTask(dataStore, indexDataID, indexInternalData, task);
			
			QueueCorridor(task.EntityDataNeedReplan, task.EntityDataCorridors, task.EntityDataCountCorridor, 
						task.EntityDataCurrentLocation, task.EntityDataPreviousLocation,
						task.EntityDataTargetLocation, task.EntityDataLastMoveLocation, task.EntityDataWaypoints, task.EntityDataCountWaypoints,
						task.EntityDataCellId, currentTime, task.EntityDataLastOptimizeTime, task.EntityDataLastMoveTargetTime);
        
			reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataNeedReplanID[indexDataID]))[indexInternalData].ValueBool = task.EntityDataNeedReplan;
			reinterpret_cast<FFluxPrimeNavigationCorridor*>(dataStore.GetRawBufferData(EntityDataCorridorsID[indexDataID]))[indexInternalData] = task.EntityDataCorridors;
			reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountCorridorID[indexDataID]))[indexInternalData].ValueUint8 = task.EntityDataCountCorridor;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataPreviousLocationID[indexDataID]))[indexInternalData] = task.EntityDataPreviousLocation;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLastMoveLocationID[indexDataID]))[indexInternalData] = task.EntityDataLastMoveLocation;
			reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataLastOptimizeTimeID[indexDataID]))[indexInternalData].ValueFloat = task.EntityDataLastOptimizeTime;
			reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataLastMoveTargetTimeID[indexDataID]))[indexInternalData].ValueFloat = task.EntityDataLastMoveTargetTime;
			reinterpret_cast<FFluxPrimeNavigationWaypoint*>(dataStore.GetRawBufferData(EntityDataWaypointsID[indexDataID]))[indexInternalData] = task.EntityDataWaypoints;
			reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountWaypointsID[indexDataID]))[indexInternalData].ValueUint8 = task.EntityDataCountWaypoints;
			reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataWaitTimeID[indexDataID]))[indexInternalData].ValueFloat = 0.0f;
			reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointLocationID[indexDataID]))[indexInternalData] = (task.EntityDataCountWaypoints > 1) ? task.EntityDataWaypoints.Waypoints[1] : task.EntityDataCurrentLocation;
		}
	}
};