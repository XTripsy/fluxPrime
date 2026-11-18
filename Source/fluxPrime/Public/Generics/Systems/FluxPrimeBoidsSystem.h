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
	/** Chunk c owns [ChunkMemberStart[c], ChunkMemberStart[c + 1]) in the arrays below. */
	TArray<int32> ChunkMemberStart;
	/** Separation kernel of every active entity, packed per archetype. */
	TArray<float> EntitySeparationDiameter;
	TArray<float> EntitySeparationRadius;
	
	UPROPERTY(EditAnywhere)
	float SeparationWeight = 0.0f;
	
	float CellSize = 100.0f;
	int32 CellWidth = 200;
	int32 CellHeight = 200;
	int32 TotalCells = 100;
	FVector Origin = FVector(-10000.0f, -10000.0f, 0.0f);
	
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
		
		CellSize = SpatialGridSystem.GetCellSize();
		CellWidth = SpatialGridSystem.GetCellWidth();
		CellHeight = SpatialGridSystem.GetCellHeight();
		TotalCells = SpatialGridSystem.GetTotalCells();
		Origin = SpatialGridSystem.GetOrigin();
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
		ChunkMemberStart.SetNumUninitialized(dataStoreCount + 1);

		int32 totalActive = 0;
		for (int32 dataIndex = 0; dataIndex < dataStoreCount; ++dataIndex)
		{
			ChunkMemberStart[dataIndex] = totalActive;
			totalActive += dataStores[dataIndex].GetActiveEntityCount();
		}
		ChunkMemberStart[dataStoreCount] = totalActive;

		EntitySeparationDiameter.SetNumUninitialized(totalActive);
		EntitySeparationRadius.SetNumUninitialized(totalActive);

		for (int32 dataIndex = 0; dataIndex < dataStoreCount; ++dataIndex)
		{
			FFluxPrimeArchetypeDataStore& dataStore = dataStores[dataIndex];

			EntityDataLocationBuffers[dataIndex] = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[dataIndex]));
			EntityDataAccelerationBuffers[dataIndex] = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataAccelerationID[dataIndex]));
			FFluxPrimeWrapperInt16* entityDataSize = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataSizeID[dataIndex]));

			check(EntityDataLocationBuffers[dataIndex]);
			check(EntityDataAccelerationBuffers[dataIndex]);
			check(entityDataSize);

			const int32 activeCount = dataStore.GetActiveEntityCount();
			if (activeCount <= 0) continue;

			float* diameters = EntitySeparationDiameter.GetData() + ChunkMemberStart[dataIndex];
			float* radii = EntitySeparationRadius.GetData() + ChunkMemberStart[dataIndex];
			for (int32 i = 0; i < activeCount; ++i)
			{
				const float diameter = entityDataSize[i].ValueInt16 * 1.2f;
				diameters[i] = diameter;
				radii[i] = FMath::Max(1.0f, FMath::Square(diameter));
			}
		}

		const TArray<int32>& gridOffsets = SpatialGridSystem.GetGridOffsets();
		const TArray<int32>& gridCounts = SpatialGridSystem.GetGridCounts();
		const FFluxPrimeGridMemberData* gridMemberData = SpatialGridSystem.GetGridMembers().GetData();

		const float* separationDiameters = EntitySeparationDiameter.GetData();
		const float* separationRadii = EntitySeparationRadius.GetData();
		const int32* chunkMemberStart = ChunkMemberStart.GetData();
		FVector* const* locationBuffers = EntityDataLocationBuffers.GetData();
		FVector* const* accelerationBuffers = EntityDataAccelerationBuffers.GetData();

	    for (int32 currentDataIdx = 0; currentDataIdx < dataStoreCount; ++currentDataIdx)
	    {
	       FVector* entityDataLocation = locationBuffers[currentDataIdx];
	       FVector* entityDataAcceleration = accelerationBuffers[currentDataIdx];
	       const int32 chunkStart = chunkMemberStart[currentDataIdx];
	       const int32 activeCount = dataStores[currentDataIdx].GetActiveEntityCount();

	       for (int32 i = 0; i < activeCount; ++i)
	       {
	          const float separationDiameter = separationDiameters[chunkStart + i];
	          const float separationRadius = separationRadii[chunkStart + i];
	          const FVector tempLocation = entityDataLocation[i];
	          FVector force = FVector::Zero();

	          const int32 agentCellX = FMath::Clamp(FMath::FloorToInt((tempLocation.X - Origin.X) / CellSize), 0, CellWidth - 1);
	          const int32 agentCellY = FMath::Clamp(FMath::FloorToInt((tempLocation.Y - Origin.Y) / CellSize), 0, CellHeight - 1);

	          // Half neighborhood: the own cell after this member plus the four cells that
	          // follow it. Every unordered pair is visited exactly once, so the interaction
	          // is computed once and written to both entities instead of twice.
	          for (int32 y = 0; y <= 1; ++y)
	          {
	             const int32 neighborY = agentCellY + y;
	             if (neighborY < 0 || neighborY >= CellHeight) continue;

	             for (int32 x = (y == 0) ? 0 : -1; x <= 1; ++x)
	             {
	                const int32 neighborX = agentCellX + x;
	                if (neighborX < 0 || neighborX >= CellWidth) continue;

	                const int32 neighborCellId = (neighborY * CellWidth) + neighborX;
	                const int32 start = gridOffsets[neighborCellId];
	                const int32 end = start + gridCounts[neighborCellId];
	                const bool isOwnCell = (x == 0 && y == 0);

	                for (int32 k = start; k < end; ++k)
	                {
	                   const int16 otherChunk = gridMemberData[k].EntityType;
	                   const int32 otherAgent = gridMemberData[k].EntityIndex;

	                   // Members are inserted in (archetype, index) order, so in the own cell
	                   // everything up to and including this member was already paired.
	                   if (isOwnCell && (otherChunk < currentDataIdx || (otherChunk == currentDataIdx && otherAgent <= i))) continue;

	                   const float diffX = tempLocation.X - gridMemberData[k].LocationX;
	                   const float diffY = tempLocation.Y - gridMemberData[k].LocationY;
	                   const float distSq = (diffX * diffX) + (diffY * diffY);

	                   if (distSq <= 0.1f) continue;

	                   const int32 otherMember = chunkMemberStart[otherChunk] + otherAgent;
	                   const float otherRadius = separationRadii[otherMember];

	                   // Neither direction is in range: skip the square root.
	                   if (distSq >= separationRadius && distSq >= otherRadius) continue;

	                   const float dist = FMath::Sqrt(distSq);

	                   if (distSq < separationRadius)
	                   {
	                      const float pushFactor = 1.0f - (dist / separationDiameter);
	                      const float scale = pushFactor / dist;

	                      force.X += diffX * scale;
	                      force.Y += diffY * scale;
	                   }

	                   if (distSq < otherRadius)
	                   {
	                      const float otherPushFactor = 1.0f - (dist / separationDiameters[otherMember]);
	                      const float otherScale = SeparationWeight * (otherPushFactor / dist);
	                      FVector& otherAcceleration = accelerationBuffers[otherChunk][otherAgent];

	                      otherAcceleration.X -= diffX * otherScale;
	                      otherAcceleration.Y -= diffY * otherScale;
	                   }
	                }
	             }
	          }

	          force *= SeparationWeight;
	          entityDataAcceleration[i] += force;
	       }
	    }
	}
};
