#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "FluxPrimeRaycastSystem.generated.h"

USTRUCT(BlueprintType)
struct FRaycastSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLocationTag;

	bool IsValid() const
	{
		return EntityDataLocationTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeRaycastSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FRaycastSystemDataTagConfig DataTagConfig;

	TArray<int16> EntityDataLocationID;
	
	UPROPERTY(EditAnywhere)
	FFluxPrimeSpatialGridSystem SpatialGridSystems = FFluxPrimeSpatialGridSystem();
	
	UPROPERTY(EditAnywhere)
	float RaycastHitRadius = 40.0f;
	
private:
	bool ShootRayGridTraversal(FVector startRay, FVector endRay, TArray<int32>& outCellIds)
	{
		FVector direction = (endRay - startRay).GetSafeNormal();
		
		const float halfCellSize = SpatialGridSystems.GetCellSize() / 2;
		float distance = FVector::Distance(startRay, endRay);
		
		int32 stepCount = FMath::CeilToInt(distance / halfCellSize);

		for (int i = 0; i < stepCount; ++i)
		{
			float stepDistance = halfCellSize * i;
			stepDistance = FMath::Min(stepDistance, distance);
			FVector forwardLocationRay = startRay + (direction * stepDistance);
			
			int32 cellId = SpatialGridSystems.GetSpatialGridSystemsCellID(forwardLocationRay);
			outCellIds.AddUnique(cellId);
		}
		
		return outCellIds.Num() > 0;
	}
	
	void GetEntitysOnCellId(const TArray<int32> cellIds, TArray<int32>& outEntityIndex, TArray<int32>& outEntityType)
	{
		outEntityType.Empty();
		outEntityIndex.Empty();
		
		for (auto& cellId : cellIds)
		{
			int32 entityCountOnCell = SpatialGridSystems.GetGridCounts()[cellId];
			
			if (entityCountOnCell <= 0) continue;
			
			int32 startOffset = SpatialGridSystems.GetGridOffsets()[cellId];

			FFluxPrimeGridMemberData tempDataMember = FFluxPrimeGridMemberData();
			for (int i = 0; i < entityCountOnCell; ++i)
			{
				tempDataMember = SpatialGridSystems.GetGridMembers()[startOffset + i];
				
				outEntityIndex.Add(tempDataMember.EntityIndex);
				outEntityType.Add(tempDataMember.EntityType);
			}
		}
	}
	
	void GetCloserEntityOnCellId(const FVector startRay, const FVector endRay, const TArray<FFluxPrimeArchetypeDataStore>& dataStores, 
		const TArray<int32> entityType, const TArray<int32> entityIndex, int32& outEntityType, int32& outEntityIndex)
	{
		float closestDistSquared = TNumericLimits<float>::Max();
		int32 shortedEntityIndex = INDEX_NONE;
		
		for (int i = 0; i < entityIndex.Num(); ++i)
		{
			FVector* entityLocations = reinterpret_cast<FVector*>(dataStores[entityType[i]].GetRawBufferData(EntityDataLocationID[entityType[i]]));
			
			check(entityLocations);
			
			FVector entityLocation = entityLocations[entityIndex[i]];
			float distToLine = FMath::PointDistToSegment(entityLocation, startRay, endRay);
			
			if (distToLine > RaycastHitRadius) continue;
			
			float distSquared = FVector::DistSquared(startRay, entityLocation);
        
			if (distSquared > closestDistSquared) continue;
			
			closestDistSquared = distSquared;
			shortedEntityIndex = i;
		}
		
		if (shortedEntityIndex == INDEX_NONE) return;
		
		outEntityType = entityType[shortedEntityIndex];
		outEntityIndex = entityIndex[shortedEntityIndex];
	}
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		for (auto& dataStore : dataStores)
		{
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
		}
		
		SpatialGridSystems.Initialized(dataStores);
	}
	
	void ShootRay(TArray<FFluxPrimeArchetypeDataStore>& dataStores, FVector startRay, FVector endRay, 
		int32& outEntityType, int32& outEntityIndex)
	{
		TArray<int32> cellIds;
		TArray<int32> entityType;
		TArray<int32> entityIndex;
		
		outEntityType = INDEX_NONE;
		outEntityIndex = INDEX_NONE;
		
		SpatialGridSystems.UpdateSpatialGridSystem(dataStores);
		
		if (!ShootRayGridTraversal(startRay, endRay, cellIds)) return;
		GetEntitysOnCellId(cellIds, entityIndex, entityType);
		GetCloserEntityOnCellId(startRay, endRay ,dataStores, entityType, entityIndex, outEntityType, outEntityIndex);
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		
	}
};
