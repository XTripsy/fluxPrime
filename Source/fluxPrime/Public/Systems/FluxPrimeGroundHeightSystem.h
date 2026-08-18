#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeBaseSystem.h"
#include "FluxPrimeGroundHeightSystem.generated.h"

#define ECC_GroundHeight ECC_GameTraceChannel1

USTRUCT(BlueprintType)
struct FFluxPrimeGroundHeightSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UWorld> world;
	
	UPROPERTY(EditAnywhere)
	float cellSize = 100.0f;

	UPROPERTY(EditAnywhere)
	FVector origin = FVector(-10000.0f, -10000.0f, 0.0f); 

	UPROPERTY(EditAnywhere)
	int32 cellWidth = 200;
	
	UPROPERTY(EditAnywhere)
	int32 cellHeight = 200;
	
	TArray<FVector>* locationCrowds = nullptr;
	TArray<float>* rotationCrowds = nullptr;
	TArray<float>* maxSpeedCrowds = nullptr;
	uint16* memberActive = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeGroundHeightSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UWorld> World;
	
	UPROPERTY(EditAnywhere)
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere)
	FVector Origin = FVector(-10000.0f, -10000.0f, 0.0f); 

	UPROPERTY(EditAnywhere)
	int32 CellWidth = 200;
	
	UPROPERTY(EditAnywhere)
	int32 CellHeight = 200;

	UPROPERTY(EditAnywhere)
	TArray<float> GroundHeightMap;
	
	TArray<FVector>* LocationCrowds = nullptr;
	TArray<float>* RotationCrowds = nullptr;
    TArray<float>* MaxSpeedCrowds = nullptr;
	uint16* MemberActive = nullptr;
	
public:
	void InitializedGroundHeightSystems(FFluxPrimeGroundHeightSystemsContext context)
	{
		check(context.world);
		check(context.memberActive);
		check(context.locationCrowds);
		check(context.rotationCrowds);
		check(context.maxSpeedCrowds);
		
		World = context.world;
		CellSize = context.cellSize;
		Origin = context.origin;
		CellWidth = context.cellWidth;
		CellHeight = context.cellHeight;
		LocationCrowds = context.locationCrowds;
		RotationCrowds = context.rotationCrowds;
		MaxSpeedCrowds = context.maxSpeedCrowds;
		MemberActive = context.memberActive;
	}
	
	void BakeGroundHeightSystems()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_GroundHeight_Systems);
		
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
				bool bHit = World->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_GroundHeight, TraceParams);
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
	
	void UpdateGroundHeightSystems(double deltaTime)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Ground_Height_Systems);
		
		FVector2D inputRange(75.0f, 300.0f);
		FVector2D outputRange(1.75f, 6.0f);
		
		auto& locationCrowds = *LocationCrowds;
		auto& rotationCrowds = *RotationCrowds;
		auto& maxSpeedCrwods = *MaxSpeedCrowds;
		
		for (int i = 0; i < *MemberActive; ++i)
		{
			float unpackedYaw = rotationCrowds[i] + 65;
			FVector forwardVector = FRotator(0.0f, unpackedYaw, 0.0f).Vector();
			FVector location = locationCrowds[i] + (forwardVector * 75);
			float value = FMath::GetMappedRangeValueClamped(inputRange, outputRange, maxSpeedCrwods[i]);
			float target = GetGroundHeight(location);
			target += 10;
			locationCrowds[i].Z = FMath::Lerp(locationCrowds[i].Z, target, deltaTime * value);
		}
	}
};
