#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeGroundHeightSystem.generated.h"

USTRUCT(BlueprintType)
struct FGroundHeightSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLocationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRotationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataMaxSpeedConfig;

	bool IsValid() const
	{
		return EntityDataLocationConfig.EntityDataTag.IsValid() 
		   && EntityDataRotationConfig.EntityDataTag.IsValid() 
		   && EntityDataMaxSpeedConfig.EntityDataTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeGroundHeightSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};


/**
* Chooses the world Z that the ground-height system should interpolate toward.
*
* Floors are the surfaces baked for one grid cell, in no particular order.
* Behavior:
* - Prefer the highest surface at or below the entity: stand on it, step down, or fall to it.
* - If every baked surface is above the entity, step up only to the closest one within
*   MaxStepHeight. This prevents a ceiling or wall top from pulling an agent upward.
* - If nothing is reachable, keep the current Z instead of snapping to an arbitrary floor.
*/
inline float SelectGroundHeightTarget(const float currentZ, const float* floors, const int32 floorCount,
	const float maxStepHeight, const float groundZOffset)
{
	if (!floors || floorCount <= 0) return currentZ;

	const float StepHeight = FMath::Max(maxStepHeight, 0.0f);
	float HighestBelow = -TNumericLimits<float>::Max();
	float ClosestAbove = TNumericLimits<float>::Max();

	for (int32 i = 0; i < floorCount; ++i)
	{
		const float FloorZ = floors[i];
		if (FloorZ <= currentZ)
		{
			HighestBelow = FMath::Max(HighestBelow, FloorZ);
		}
		else if (FloorZ <= currentZ + StepHeight)
		{
			ClosestAbove = FMath::Min(ClosestAbove, FloorZ);
		}
	}

	if (HighestBelow > -TNumericLimits<float>::Max())
	{
		return HighestBelow + groundZOffset;
	}

	if (ClosestAbove < TNumericLimits<float>::Max())
	{
		return ClosestAbove + groundZOffset;
	}

	return currentZ;
}

USTRUCT(BlueprintType)
struct FFluxPrimeGroundHeightSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FGroundHeightSystemDataTagConfig DataTagConfig;

	TArray<int16> EntityDataLocationID, EntityDataRotationID, EntityDataMaxSpeedID;
	
	UPROPERTY(EditAnywhere)
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, Category = "Ground Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_GameTraceChannel1;

	UPROPERTY(EditAnywhere)
	FVector Origin = FVector(-10000.0f, -10000.0f, 0.0f); 

	UPROPERTY(EditAnywhere)
	int32 CellWidth = 200;
	
	UPROPERTY(EditAnywhere)
	int32 CellHeight = 200;
	
	UPROPERTY(EditAnywhere)
	float MaxStepHeight = 100.0f;

	UPROPERTY(EditAnywhere)
	float MinFloorSeparation = 120.0f;

	UPROPERTY(EditAnywhere)
	float GroundZOffset = 10.0f;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", ClampMax = "16"))
	int32 MaxFloorsPerCell = 1;
	
	TArray<float> GroundHeightMap;
	TArray<uint8> CellFloorCounts;
	
private:
	void BakeGroundHeightSystems(TObjectPtr<UWorld> world)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_GroundHeight_Systems);
		
		MaxFloorsPerCell = FMath::Max(1, MaxFloorsPerCell);
		int32 totalCells = CellWidth * CellHeight;

		GroundHeightMap.Init(0.0f, totalCells * MaxFloorsPerCell);
		CellFloorCounts.Init(0, totalCells);

		FCollisionQueryParams traceParams;
		traceParams.bTraceComplex = false;

		const float maxWorldZ = 10000.0f;
		const float minWorldZ = -10000.0f;
		const float penetrationStepZ = 30.0f;

		for (int32 y = 0; y < CellHeight; y++)
		{
			for (int32 x = 0; x < CellWidth; x++)
			{
				float worldX = Origin.X + (x * CellSize) + (CellSize * 0.5f);
				float worldY = Origin.Y + (y * CellSize) + (CellSize * 0.5f);

				int32 cellIndex = (y * CellWidth) + x;
				int32 baseDataIndex = cellIndex * MaxFloorsPerCell;
				uint8 currentFloorCount = 0;

				float currentRayStartZ = maxWorldZ;

				while (currentRayStartZ > minWorldZ && currentFloorCount < MaxFloorsPerCell)
				{
					FVector rayStart(worldX, worldY, currentRayStartZ);
					FVector rayEnd(worldX, worldY, minWorldZ);

					FHitResult hitResult;
					bool bHit = world->LineTraceSingleByChannel(hitResult, rayStart, rayEnd, TraceChannel, traceParams);

					if (!bHit) break;

					if (hitResult.bStartPenetrating)
					{
						currentRayStartZ -= penetrationStepZ;
						continue;
					}

					if (hitResult.ImpactNormal.Z >= 0.7f)
					{
						bool bTooCloseToUpperFloor = false;
						if (currentFloorCount > 0)
						{
							float prevFloorZ = GroundHeightMap[baseDataIndex + currentFloorCount - 1];
							if (FMath::Abs(prevFloorZ - hitResult.ImpactPoint.Z) < MinFloorSeparation) 
								bTooCloseToUpperFloor = true;
						}

						if (!bTooCloseToUpperFloor)
						{
							GroundHeightMap[baseDataIndex + currentFloorCount] = hitResult.ImpactPoint.Z;
							currentFloorCount++;
						}
					}

					currentRayStartZ = hitResult.ImpactPoint.Z - penetrationStepZ;
				}

				CellFloorCounts[cellIndex] = currentFloorCount;
			}
		}
	}
	
	float GetGroundHeight(const FVector& location)
	{
		int32 cellX = FMath::FloorToInt((location.X - Origin.X) / CellSize);
		int32 cellY = FMath::FloorToInt((location.Y - Origin.Y) / CellSize);

		cellX = FMath::Clamp(cellX, 0, CellWidth - 1);
		cellY = FMath::Clamp(cellY, 0, CellHeight - 1);

		int32 cellIndex = (cellY * CellWidth) + cellX;
		uint8 floorCount = CellFloorCounts[cellIndex];

		if (floorCount == 0) return location.Z;

		int32 baseDataIndex = cellIndex * MaxFloorsPerCell;
		return SelectGroundHeightTarget(location.Z, &GroundHeightMap[baseDataIndex], floorCount,
			MaxStepHeight, GroundZOffset);
	}
	
public:
	void Initialized(FFluxPrimeGroundHeightSystemsContext context)
	{
		check(context.world);

		int32 countData = context.dataStores->Num();
		EntityDataLocationID.Init(0, countData);
		EntityDataRotationID.Init(0, countData);
		EntityDataMaxSpeedID.Init(0, countData);
		
		int32 indexDataId = 0;
		for (auto& dataStore : *context.dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataLocationConfig, EntityDataLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRotationConfig, EntityDataRotationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataMaxSpeedConfig, EntityDataMaxSpeedID[indexDataId]);
			indexDataId++;
		}
		
		BakeGroundHeightSystems(context.world);
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Ground_Height_Systems);
		
		if (!IsAllDataValid)
		{
			UE_LOG(LogFluxPrime, Verbose, TEXT("GROUND HEIGHT SYSTEM :: data invalid; system skipped"));
			return;
		}
		
		const FVector2D inputRange(75.0f, 300.0f);
		const FVector2D outputRange(1.75f, 6.0f);
		
		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
			FFluxPrimeWrapperFloat* entityDataMaxSpeed = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataMaxSpeedID[indexDataID]));
			
			check(entityDataLocation);
			check(entityDataMaxSpeed);
			
			int32 activeCount = dataStore.GetActiveEntityCount();
			for (int32 i = 0; i < activeCount; ++i)
			{
				float interpMultiplier = FMath::GetMappedRangeValueClamped(inputRange, outputRange, entityDataMaxSpeed[i].ValueFloat);
				float targetZ = GetGroundHeight(entityDataLocation[i]);
				
				entityDataLocation[i].Z = FMath::Lerp(entityDataLocation[i].Z, targetZ, deltaTime * interpMultiplier);
			}
			
			indexDataID++;
		}
	}
};