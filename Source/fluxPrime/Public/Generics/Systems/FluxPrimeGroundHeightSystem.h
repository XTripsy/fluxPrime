#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "FluxPrimeGroundHeightSystem.generated.h"

#define ECC_GroundHeight ECC_GameTraceChannel1

USTRUCT(BlueprintType)
struct FGroundHeightSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRotationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataMaxSpeedTag;

	bool IsValid() const
	{
		return EntityDataLocationTag.IsValid() 
		   && EntityDataRotationTag.IsValid() 
		   && EntityDataMaxSpeedTag.IsValid();
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

	UPROPERTY(EditAnywhere)
	FVector Origin = FVector(-10000.0f, -10000.0f, 0.0f); 

	UPROPERTY(EditAnywhere)
	int32 CellWidth = 200;
	
	UPROPERTY(EditAnywhere)
	int32 CellHeight = 200;
	
	UPROPERTY()
	TArray<float> GroundHeightMap;
	
private:
	void BakeGroundHeightSystems(TObjectPtr<UWorld> world)
	{
#if IF_WITH_EDITOR
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_GroundHeight_Systems);
#endif
		
		int32 TotalCells = CellWidth * CellHeight;
		GroundHeightMap.Init(0.0f, TotalCells);

		FCollisionQueryParams TraceParams;
		TraceParams.bTraceComplex = false;

		for (int32 y = 0; y < CellHeight; y++)
		{
			for (int32 x = 0; x < CellWidth; x++)
			{
				float WorldX = Origin.X + (x * CellSize) + (CellSize * 0.5f);
				float WorldY = Origin.Y + (y * CellSize) + (CellSize * 0.5f);

				FVector RayStart(WorldX, WorldY, 10000.0f);
				FVector RayEnd(WorldX, WorldY, -10000.0f);

				FHitResult HitResult;
				bool bHit = world->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_GroundHeight, TraceParams);
				int32 IndexID = (y * CellWidth) + x;

				GroundHeightMap[IndexID] = (bHit) ? HitResult.ImpactPoint.Z : 0.0f;
			}
		}
	}
	
	float GetGroundHeight(FVector location)
	{
		int32 CellX = FMath::FloorToInt((location.X - Origin.X) / CellSize);
		int32 CellY = FMath::FloorToInt((location.Y - Origin.Y) / CellSize);

		CellX = FMath::Clamp(CellX, 0, CellWidth - 1);
		CellY = FMath::Clamp(CellY, 0, CellHeight - 1);

		int32 IndexID = (CellY * CellWidth) + CellX;

		return GroundHeightMap[IndexID];
	}
	
public:
	void Initialized(FFluxPrimeGroundHeightSystemsContext context)
	{
		check(context.world);

		for (auto& dataStore : *context.dataStores)
		{
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataRotationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRotationTag));
			EntityDataMaxSpeedID.Add(dataStore.GetDataID(DataTagConfig.EntityDataMaxSpeedTag));
		}
		
		BakeGroundHeightSystems(context.world);
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Ground_Height_Systems);
		
		FVector2D inputRange(75.0f, 300.0f);
		FVector2D outputRange(1.75f, 6.0f);
		
		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
			FRotator* entityDataRotation = reinterpret_cast<FRotator*>(dataStore.GetRawBufferData(EntityDataRotationID[indexDataID]));
			FFluxPrimeWrapperFloat* entityDataMaxSpeed = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataMaxSpeedID[indexDataID]));
			
			for (int i = 0; i < dataStore.GetActiveEntityCount(); ++i)
			{
				float unpackedYaw = entityDataRotation[i].Yaw + 65;
				FVector forwardVector = FRotator(0.0f, unpackedYaw, 0.0f).Vector();
				FVector tempLocation = entityDataLocation[i] + (forwardVector * 75);
				float value = FMath::GetMappedRangeValueClamped(inputRange, outputRange, entityDataMaxSpeed[i].ValueFloat);
				float target = GetGroundHeight(tempLocation);
				target += 10;
				entityDataLocation[i].Z = FMath::Lerp(entityDataLocation[i].Z, target, deltaTime * value);
			}
			
			indexDataID++;
		}
	}
};