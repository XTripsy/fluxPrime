#pragma once

#include "CoreMinimal.h"
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
	FGameplayTag EntityDataLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataAccelerationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataSizeTag;

	bool IsValid() const
	{
		return EntityDataLocationTag.IsValid() 
		   && EntityDataAccelerationTag.IsValid() 
		   && EntityDataSizeTag.IsValid();
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
		for (auto& dataStore : dataStores)
		{
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataAccelerationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataAccelerationTag));
			EntityDataSizeID.Add(dataStore.GetDataID(DataTagConfig.EntityDataSizeTag));
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

	    SpatialGridSystem.UpdateSpatialGridSystem(dataStores);

	    for (int32 currentDataIdx = 0; currentDataIdx < dataStores.Num(); ++currentDataIdx)
	    {
	       auto& currentDataStore = dataStores[currentDataIdx];

	       FVector* entityDataLocation = reinterpret_cast<FVector*>(currentDataStore.GetRawBufferData(EntityDataLocationID[currentDataIdx]));
	       FVector* entityDataAcceleration = reinterpret_cast<FVector*>(currentDataStore.GetRawBufferData(EntityDataAccelerationID[currentDataIdx]));
	       FFluxPrimeWrapperInt16* entityDataSize = reinterpret_cast<FFluxPrimeWrapperInt16*>(currentDataStore.GetRawBufferData(EntityDataSizeID[currentDataIdx]));

	       check(entityDataLocation);
	       check(entityDataAcceleration);
	       check(entityDataSize);
	       
	       for (int32 i = 0; i < currentDataStore.GetActiveEntityCount(); ++i)
	       {
	          float separationRadius = FMath::Square(entityDataSize[i].ValueInt16 * 1.2f);
	          separationRadius = FMath::Max(1.0f, separationRadius);
	          FVector tempLocation = entityDataLocation[i];
	          FVector force = FVector::Zero();
	          int32 overlapCount = 0;

	          int32 agentCellX = FMath::FloorToInt((tempLocation.X - Origin.X) / CellSize);
	          int32 agentCellY = FMath::FloorToInt((tempLocation.Y - Origin.Y) / CellSize);

	          for (int8 x = -1; x <= 1; ++x)
	          {
	             for (int8 y = -1; y <= 1; ++y)
	             {
	                int32 neighborX = agentCellX + x;
	                int32 neighborY = agentCellY + y;
	                
	                if (neighborX < 0 || neighborX >= CellWidth || neighborY < 0 || neighborY >= CellHeight) continue;

	                int32 neighborCellId = (neighborY * CellWidth) + neighborX;

	                TArray<int32>& gridOffsets = SpatialGridSystem.GetGridOffsets();
	                TArray<int32>& gridCounts = SpatialGridSystem.GetGridCounts();
	                
	                TArray<FFluxPrimeGridMemberData>& gridMembers = SpatialGridSystem.GetGridMembers();

	                int32 start = gridOffsets[neighborCellId];
	                int32 count = gridCounts[neighborCellId];

	                for(int32 k = 0; k < count; ++k)
	                {
	                   int16 otherChunk = gridMembers[start + k].EntityType;
	                   int32 otherAgent = gridMembers[start + k].EntityIndex;

	                   if (currentDataIdx == otherChunk && i == otherAgent) continue;

	                   FVector* otherLocations = reinterpret_cast<FVector*>(dataStores[otherChunk].GetRawBufferData(EntityDataLocationID[otherChunk]));

	                   FVector diff = tempLocation - otherLocations[otherAgent];
	                   diff.Z = 0;
	                   float distSq = diff.SizeSquared();

	                   if (distSq < separationRadius && distSq > 0.1f)
	                   {
	                      float dist = FMath::Sqrt(distSq);
	                      float pushFactor = 1.0f - (dist / (entityDataSize[i].ValueInt16 * 1.2f));

	                      force += (diff / dist) * pushFactor;
	                      overlapCount++;
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
