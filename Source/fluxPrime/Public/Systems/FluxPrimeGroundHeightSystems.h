#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeBaseSystems.h"
#include "FluxPrimeGroundHeightSystems.generated.h"

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
	
	//TStaticArray<FFluxPrimeCrowds, 2>* members = nullptr;
	FFluxPrimeCrowds* members = nullptr;
	//int8* dataReadIndex = nullptr;
	uint16* memberActive = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeGroundHeightSystems : public FFluxPrimeBaseSystems
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
	
	//TStaticArray<FFluxPrimeCrowds, 2>* Members = nullptr;
	FFluxPrimeCrowds* Members = nullptr;
	//int8* DataReadIndex = nullptr;
	uint16* MemberActive = nullptr;
	
public:
	void InitializedGroundHeightSystems(FFluxPrimeGroundHeightSystemsContext context)
	{
		check(context.world);
		check(context.members);
		check(context.memberActive);
		//check(context.dataReadIndex);
		
		World = context.world;
		CellSize = context.cellSize;
		Origin = context.origin;
		CellWidth = context.cellWidth;
		CellHeight = context.cellHeight;
		Members = context.members;
		MemberActive = context.memberActive;
		//DataReadIndex = context.dataReadIndex;
	}
	
	void BakeGroundHeightSystems()
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
		//auto& member = (*Members)[*DataReadIndex];
		auto& member = *Members;
		
		FVector2D inputRange(75.0f, 300.0f);
		FVector2D outputRange(1.75f, 6.0f);
		
		for (int i = 0; i < *MemberActive; ++i)
		{
			float unpackedYaw = member.CrowdsRotation[i] + 65;//FRotator::DecompressAxisFromByte(members.CrowdsRotation[i]) + 65;
			FVector forwardVector = FRotator(0.0f, unpackedYaw, 0.0f).Vector();
			FVector location = member.CrowdsLocation[i] + (forwardVector * 75);
			float value = FMath::GetMappedRangeValueClamped(inputRange, outputRange, member.CrowdsMaxSpeed[i]);
			float target = GetGroundHeight(location);
			target += 10;
			member.CrowdsLocation[i].Z = FMath::Lerp(member.CrowdsLocation[i].Z, target, deltaTime * value);
		}
	}
	
	void UpdateNetGroundHeightSystems(double deltaTime, TArray<FFluxPrimeCrowdsNet>& members, const int32 memberActive)
	{
		FVector2D inputRange(75.0f, 300.0f);
		FVector2D outputRange(1.75f, 6.0f);
		
		for (int i = 0; i < memberActive; ++i)
		{
			float unpackedYaw = FRotator::DecompressAxisFromByte(members[i].NetRotation) + 65;
			FVector forwardVector = FRotator(0.0f, unpackedYaw, 0.0f).Vector();
			FVector location = members[i].NetLocation + (forwardVector * 75);
			float value = FMath::GetMappedRangeValueClamped(inputRange, outputRange, members[i].NetMaxSpeed);
			float target = GetGroundHeight(location);
			target += 10;
			members[i].NetLocation.Z = FMath::Lerp(members[i].NetLocation.Z, target, deltaTime * value);
		}
	}
};
