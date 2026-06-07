#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystems.h"
#include "Cores/FluxPrimeStruct.h"
#include "Kismet/GameplayStatics.h"
#include "FluxPrimeBoidsSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeBoidsSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()
	
private:
	UPROPERTY(VisibleAnywhere)
	float SeparationWeight = 0.0f;
	
	UPROPERTY(EditAnywhere)
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere)
	FVector Origin = FVector(-10000.0f, -10000.0f, 0.0f); 

	UPROPERTY(EditAnywhere)
	int32 CellWidth = 200;
	
	UPROPERTY(EditAnywhere)
	int32 CellHeight = 200;
    
	UPROPERTY(EditAnywhere)
	int32 TotalCells = 100;
	
	//FVector playerLocation;
	
public:
	void InitializeBoidsSystems(float separationWeight, float cellSize, FVector origin, int32 cellWidth, int32 cellHeight)
	{
		SeparationWeight = separationWeight;
		CellSize = cellSize;
		Origin = origin;
		CellWidth = cellWidth;
		CellHeight = cellHeight;
	}
	
	void UpdateBoidsSystems(/*TObjectPtr<UWorld> world,*/ FFluxPrimeCrowds& members, const TArray<int32>& gridOffset, int32 activeMember)
	{
		//playerLocation = UGameplayStatics::GetPlayerPawn(world, 0)->GetActorLocation();
		
		for (int32 i = 0; i < activeMember; ++i)
		{
			float separationRadius = FMath::Square(members.CrowdsSize[i] * 1.2f);
			FVector location = members.CrowdsLocation[i];
			FVector force = FVector::Zero();
			int overlapCount = 0;
			
			int32 agentCellX = FMath::FloorToInt((location.X - Origin.X) / CellSize);
			int32 agentCellY = FMath::FloorToInt((location.Y - Origin.Y) / CellSize);
			
			/*FVector diffToPlayer = location - playerLocation;
			diffToPlayer.Z = 0;
			float distToPlayerSq = diffToPlayer.SizeSquared();

			if (distToPlayerSq < separationRadius && distToPlayerSq > 0.1f)
			{
				float distToPlayer = FMath::Sqrt(distToPlayerSq);
				float pushFactor = 1.0f - (distToPlayer / separationRadius);
				FVector evasionForce = (diffToPlayer / distToPlayer) * pushFactor * separationRadius;
				members.CrowdsAcceleration[i] += evasionForce;
			}*/
			
			for (int x = -1; x <= 1; ++x)
			{
				for (int y = -1; y <= 1; ++y)
				{
					int32 neighborX = agentCellX + x;
					int32 neighborY = agentCellY + y;
					
					if (neighborX < 0 || neighborX >= CellWidth || neighborY < 0 || neighborY >= CellHeight) continue;
					
					int32 neighborCellId = (neighborY * CellWidth) + neighborX;
					
					int32 startIndex = gridOffset[neighborCellId];
					if (startIndex == -1) continue;
					
					for (int j = startIndex; j < activeMember; ++j)
					{
						if (members.CrowdsCellID[j] != neighborCellId) break;
						if (i == j) continue;
						
						FVector diff = location - members.CrowdsLocation[j];
						diff.Z = 0;
						float distSq = diff.SizeSquared();
						
						if (distSq < separationRadius && distSq > 0.1f)
						{
							float dist = FMath::Sqrt(distSq);
							float pushFactor = 1.0f - (dist / (members.CrowdsSize[i] * 1.2f));
							
							force += (diff / dist) * pushFactor;
							overlapCount++;
						}
					}
				}
			}
			
			if (overlapCount > 0) force /= overlapCount;
			force *= SeparationWeight;
			members.CrowdsAcceleration[i] += force;
		}
	}
};
