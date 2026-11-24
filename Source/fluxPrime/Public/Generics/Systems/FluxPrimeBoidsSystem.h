#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "FluxPrimeSpatialGridSystem.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeBoidsSystem.generated.h"

USTRUCT(BlueprintType)
struct FBoidsSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLocationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataAccelerationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataSizeConfig;

	bool IsValid() const
	{
		return EntityDataLocationConfig.EntityDataTag.IsValid() 
		   && EntityDataAccelerationConfig.EntityDataTag.IsValid() 
		   && EntityDataSizeConfig.EntityDataTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeBoidsSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FBoidsSystemDataTagConfig DataTagConfig;

	TArray<int16> EntityDataLocationID, EntityDataAccelerationID, EntityDataSizeID;
	/** Buffer pointers of each archetype, resolved once per frame instead of per neighbor. */
	TArray<FVector*> EntityDataLocationBuffers;
	TArray<FVector*> EntityDataAccelerationBuffers;
	TArray<FFluxPrimeWrapperInt16*> EntityDataSizeBuffers;
	/** Separation kernel of every active entity, packed in grid member order. */
	TArray<float> EntitySeparationDiameter;
	TArray<float> EntitySeparationRadius;
	
	UPROPERTY(EditAnywhere)
	float SeparationWeight = 0.0f;
	
	int32 CellWidth = 200;
	int32 CellHeight = 200;
	
	UPROPERTY(EditAnywhere)
	FFluxPrimeSpatialGridSystem SpatialGridSystem = FFluxPrimeSpatialGridSystem();
	//FVector playerLocation;
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		int32 countData = dataStores.Num();
		EntityDataLocationID.Init(0, countData);
		EntityDataAccelerationID.Init(0, countData);
		EntityDataSizeID.Init(0, countData);
		
		int32 indexDataId = 0;
		for (auto& dataStore : dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataLocationConfig, EntityDataLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataAccelerationConfig, EntityDataAccelerationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataSizeConfig, EntityDataSizeID[indexDataId]);
			indexDataId++;
		}
		
		SpatialGridSystem.Initialized(dataStores);
		
		CellWidth = SpatialGridSystem.GetCellWidth();
		CellHeight = SpatialGridSystem.GetCellHeight();
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
	    TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Boids_Systems);
		
		if (!IsAllDataValid)
		{
			UE_LOG(LogFluxPrime, Verbose, TEXT("BOIDS SYSTEM :: data invalid; system skipped"));
			return;
		}

	    SpatialGridSystem.UpdateSpatialGridSystem(dataStores);

		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Boids_NeighborScan);

		// Resolve buffers once per tick so compaction/reallocation cannot leave stale pointers.
		const int32 dataStoreCount = dataStores.Num();
		EntityDataLocationBuffers.SetNumUninitialized(dataStoreCount);
		EntityDataAccelerationBuffers.SetNumUninitialized(dataStoreCount);
		EntityDataSizeBuffers.SetNumUninitialized(dataStoreCount);

		int32 totalActive = 0;
		for (int32 dataIndex = 0; dataIndex < dataStoreCount; ++dataIndex)
		{
			FFluxPrimeArchetypeDataStore& dataStore = dataStores[dataIndex];

			EntityDataLocationBuffers[dataIndex] = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[dataIndex]));
			EntityDataAccelerationBuffers[dataIndex] = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataAccelerationID[dataIndex]));
			EntityDataSizeBuffers[dataIndex] = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataSizeID[dataIndex]));

			check(EntityDataLocationBuffers[dataIndex]);
			check(EntityDataAccelerationBuffers[dataIndex]);
			check(EntityDataSizeBuffers[dataIndex]);

			totalActive += dataStore.GetActiveEntityCount();
		}

		const TArray<int32>& gridOffsets = SpatialGridSystem.GetGridOffsets();
		const TArray<int32>& gridCounts = SpatialGridSystem.GetGridCounts();
		const FFluxPrimeGridMemberData* gridMemberData = SpatialGridSystem.GetGridMembers().GetData();
		const FFluxPrimeWrapperInt16* const* sizeBuffers = EntityDataSizeBuffers.GetData();

		EntitySeparationDiameter.SetNumUninitialized(totalActive);
		EntitySeparationRadius.SetNumUninitialized(totalActive);

		// Packed in grid member order, which is also the order the scan below walks: the
		// candidate index is then the separation index, with no per-pair archetype lookup.
		check(SpatialGridSystem.GetGridMembers().Num() >= totalActive);
		float* diameters = EntitySeparationDiameter.GetData();
		float* radii = EntitySeparationRadius.GetData();
		for (int32 member = 0; member < totalActive; ++member)
		{
			const float diameter = sizeBuffers[gridMemberData[member].EntityType][gridMemberData[member].EntityIndex].ValueInt16 * 1.2f;
			diameters[member] = diameter;
			radii[member] = FMath::Max(1.0f, FMath::Square(diameter));
		}

		const float* separationDiameters = EntitySeparationDiameter.GetData();
		const float* separationRadii = EntitySeparationRadius.GetData();
		FVector* const* locationBuffers = EntityDataLocationBuffers.GetData();
		FVector* const* accelerationBuffers = EntityDataAccelerationBuffers.GetData();

		const TArray<int32>& occupiedCells = SpatialGridSystem.GetOccupiedCells();

		// Occupied cells only: each member is the "self" of its own pair set. Its pairs are the
		// strictly later members of its own cell, every member of the cell to its right, and
		// the three cells in the next row. Members are inserted in (archetype, row) order, so
		// "strictly later in this cell" is exactly the set the previous per-entity skip loop
		// walked one rejection at a time, and the left cell needs no visit because its own
		// right-cell pass already pairs it with this one.
		for (const int32 cellId : occupiedCells)
		{
			const int32 cellStart = gridOffsets[cellId];
			const int32 cellEnd = cellStart + gridCounts[cellId];
			const int32 cellX = cellId % CellWidth;
			const int32 cellY = cellId / CellWidth;
			const int32 rightCellId = (cellX + 1 < CellWidth) ? cellId + 1 : INDEX_NONE;
			const int32 forwardRowStart = (cellY + 1) * CellWidth;
			const int32 rightStart = rightCellId == INDEX_NONE ? 0 : gridOffsets[rightCellId];
			const int32 rightEnd = rightCellId == INDEX_NONE ? 0 : rightStart + gridCounts[rightCellId];
			int32 forwardStarts[3];
			int32 forwardEnds[3];
			int32 forwardCount = 0;
			if (cellY + 1 < CellHeight)
			{
				for (int32 neighborX = FMath::Max(cellX - 1, 0); neighborX <= FMath::Min(cellX + 1, CellWidth - 1); ++neighborX)
				{
					const int32 neighborCellId = forwardRowStart + neighborX;
					if (gridCounts[neighborCellId] == 0) continue;
					forwardStarts[forwardCount] = gridOffsets[neighborCellId];
					forwardEnds[forwardCount] = forwardStarts[forwardCount] + gridCounts[neighborCellId];
					++forwardCount;
				}
			}

			for (int32 m = cellStart; m < cellEnd; ++m)
			{
				const int16 selfChunk = gridMemberData[m].EntityType;
				const int32 selfAgent = gridMemberData[m].EntityIndex;
				const FVector selfLocation = locationBuffers[selfChunk][selfAgent];
				const float selfDiameter = separationDiameters[m];
				const float selfRadius = separationRadii[m];
				FVector force = FVector::Zero();

				const auto ApplyPair = [&](const int32 otherMemberIndex)
				{
					const float diffX = selfLocation.X - gridMemberData[otherMemberIndex].LocationX;
					const float diffY = selfLocation.Y - gridMemberData[otherMemberIndex].LocationY;
					const float distSq = (diffX * diffX) + (diffY * diffY);

					if (distSq <= 0.1f) return;

					const float otherRadius = separationRadii[otherMemberIndex];

					// Neither direction is in range: skip the square root.
					if (distSq >= selfRadius && distSq >= otherRadius) return;

					const float dist = FMath::Sqrt(distSq);

					if (distSq < selfRadius)
					{
						const float pushFactor = 1.0f - (dist / selfDiameter);
						const float scale = pushFactor / dist;

						force.X += diffX * scale;
						force.Y += diffY * scale;
					}

					if (distSq < otherRadius)
					{
						const float otherPushFactor = 1.0f - (dist / separationDiameters[otherMemberIndex]);
						const float otherScale = SeparationWeight * (otherPushFactor / dist);
						FVector& otherAcceleration = accelerationBuffers[gridMemberData[otherMemberIndex].EntityType][gridMemberData[otherMemberIndex].EntityIndex];

						otherAcceleration.X -= diffX * otherScale;
						otherAcceleration.Y -= diffY * otherScale;
					}
				};

				for (int32 k = m + 1; k < cellEnd; ++k)
				{
					ApplyPair(k);
				}

				for (int32 k = rightStart; k < rightEnd; ++k)
				{
					ApplyPair(k);
				}

				for (int32 range = 0; range < forwardCount; ++range)
				{
					for (int32 k = forwardStarts[range]; k < forwardEnds[range]; ++k)
					{
						ApplyPair(k);
					}
				}

				force *= SeparationWeight;
				accelerationBuffers[selfChunk][selfAgent] += force;
			}
		}
	}
};
