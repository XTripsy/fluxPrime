#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeGroundHeightSystems.generated.h"

#define ECC_GroundHeight ECC_GameTraceChannel1

USTRUCT(BlueprintType)
struct FFluxPrimeGroundHeightSystems
{
	GENERATED_BODY()
	
private:
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
	
public:
	void InitializedGroundHeightSystems(float cellSize, FVector origin, int32 cellWidth, int32 cellHeight)
	{
		CellSize = cellSize;
		Origin = origin;
		CellWidth = cellWidth;
		CellHeight = cellHeight;
	}
	
	void BakeGroundHeightSystems(TObjectPtr<UWorld> world)
	{
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
				int32 Index1D = (y * CellWidth) + x;

				GroundHeightMap[Index1D] = (bHit) ? HitResult.ImpactPoint.Z : 0.0f;
			}
		}
	}
	
	float GetGroundHeight(FVector location)
	{
		int32 CellX = FMath::FloorToInt((location.X - Origin.X) / CellSize);
		int32 CellY = FMath::FloorToInt((location.Y - Origin.Y) / CellSize);

		CellX = FMath::Clamp(CellX, 0, CellWidth - 1);
		CellY = FMath::Clamp(CellY, 0, CellHeight - 1);

		int32 Index1D = (CellY * CellWidth) + CellX;

		return GroundHeightMap[Index1D];
	}
	
	void UpdateGroundHeightSystems(double deltaTime, FFluxPrimeCrowds& members, const int32 memberActive)
	{
		FVector2D inputRange(75.0f, 300.0f);
		FVector2D outputRange(1.75f, 6.0f);
		
		for (int i = 0; i < memberActive; ++i)
		{
			float unpackedYaw = FRotator::DecompressAxisFromByte(members.CrowdsRotation[i]) + 65;
			FVector ForwardVector = FRotator(0.0f, unpackedYaw, 0.0f).Vector();
			FVector location = members.CrowdsLocation[i] + (ForwardVector * 75);
			float value = FMath::GetMappedRangeValueClamped(inputRange, outputRange, members.CrowdsMaxSpeed[i]);
			float target = GetGroundHeight(location);
			target += 10;
			members.CrowdsLocation[i].Z = FMath::Lerp(members.CrowdsLocation[i].Z, target, deltaTime * value);
		}
	}
};
