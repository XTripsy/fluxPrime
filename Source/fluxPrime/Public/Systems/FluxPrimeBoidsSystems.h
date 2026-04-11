#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeBoidsSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeBoidsSystems
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	float SeparationWeight = 0.0f;
	
	void InitializeBoidsSystems(float separationWeight)
	{
		SeparationWeight = separationWeight;
	}
	
	void UpdateBoidsSystems(FFluxPrimeCrowds& members, const TArray<int32>& gridOffset, float cellSize, int32 totalCells, int32 activeMember)
	{
		for (int32 i = 0; i < activeMember; ++i)
		{
			float separationRadius = FMath::Square(members.CrowdsSize[i]);
			FVector location = members.CrowdsLocation[i];
			FVector force = FVector::Zero();
			
			int32 gridX = FMath::FloorToInt(location.X / cellSize);
			int32 gridY = FMath::FloorToInt(location.Y / cellSize);

			for (int x = -1; x <= 1; ++x)
			{
				for (int y = -1; y <= 1; ++y)
				{
					int32 neighborX = gridX + x;
					int32 neighborY = gridY + y;
					
					int32 hash = (neighborX * 73856093) ^ (neighborY * 193496629);
					int32 neighborCellId = FMath::Abs(hash) % totalCells;
					int32 startIndex = gridOffset[neighborCellId];
					
					if (startIndex == -1) continue;

					for (int j = startIndex; j < activeMember; ++j)
					{
						if (members.CrowdsCellID[j] != neighborCellId) break;
						if (i == j) continue;
						
						FVector diff = location - members.CrowdsLocation[j];
						float distSq = diff.SizeSquared();
             
						float actualDist = FMath::Sqrt(distSq);
						float pushIntensity = (separationRadius - actualDist) / separationRadius;
						force += (diff.GetSafeNormal() * (pushIntensity * 1000.0f)) * (distSq < separationRadius && distSq > 0.01f);
					}
				}
			}
			
			members.CrowdsAcceleration[i] += force * SeparationWeight;
		}
	}
};
