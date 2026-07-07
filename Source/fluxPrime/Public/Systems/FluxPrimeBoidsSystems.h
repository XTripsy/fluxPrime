#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystems.h"
#include "FluxPrimeSpatialGridSystems.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeBoidsSystems.generated.h"

USTRUCT()
struct FFluxPrimeBoidsSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	float separationWeight = 0.0f;
	
	UPROPERTY(EditAnywhere)
	FFluxPrimeSpatialGridSystemsContext contextSpatialGrid;
};

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
	
	//TStaticArray<FFluxPrimeCrowds, 2>* Members = nullptr;
	FFluxPrimeCrowds* Members = nullptr;
	/*int8* DataReadIndex = nullptr;
	TArray<int32>* GridOffset = nullptr;*/
	uint16* MemberActive = nullptr;
	
	FFluxPrimeSpatialGridSystems SpatialGridSystems;
	
	//FVector playerLocation;
	
public:
	void InitializeBoidsSystems(FFluxPrimeBoidsSystemsContext context)
	{
		check(context.contextSpatialGrid.members);
		check(context.contextSpatialGrid.memberActive);
		/*check(context.contextSpatialGrid.gridOffset);
		check(context.contextSpatialGrid.dataReadIndex);*/
		
		SeparationWeight = context.separationWeight;
		CellSize = context.contextSpatialGrid.cellSize;
		Origin = context.contextSpatialGrid.origin;
		CellWidth = context.contextSpatialGrid.cellWidth;
		CellHeight = context.contextSpatialGrid.cellHeight;
		Members = context.contextSpatialGrid.members;
		//GridOffset = context.contextSpatialGrid.gridOffset;
		MemberActive = context.contextSpatialGrid.memberActive;
		//DataReadIndex = context.contextSpatialGrid.dataReadIndex;
		
		SpatialGridSystems.InitializedSpatialGridSystems(context.contextSpatialGrid);
		SpatialGridSystems.BakeSpatialGridSystems();
	}
	
	void UpdateBoidsSystems()
	{
		//playerLocation = UGameplayStatics::GetPlayerPawn(world, 0)->GetActorLocation();
		
		SpatialGridSystems.UpdateSpatialGridSystem();
		
		//auto& members = (*Members)[*DataReadIndex];
		auto& members = *Members;
		
		TArray<int32>& gridOffsets = SpatialGridSystems.GetGridOffsets();
		TArray<int32>& gridCounts = SpatialGridSystems.GetGridCounts();
		TArray<int32>& gridMembers = SpatialGridSystems.GetGridMembers();
		
		for (int32 i = 0; i < *MemberActive; ++i)
		{
			float separationRadius = FMath::Square(members.CrowdsSize[i] * 1.2f);
			FVector location = members.CrowdsLocation[i];
			FVector force = FVector::Zero();
			int32 overlapCount = 0;
			
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
			
			//for (int x = -1; x <= 1; ++x)
			for (int8 x = -1; x <= 1; ++x)
			{
				//for (int y = -1; y <= 1; ++y)
				for (int8 y = -1; y <= 1; ++y)
				{
					int32 neighborX = agentCellX + x;
					int32 neighborY = agentCellY + y;
					
					if (neighborX < 0 || neighborX >= CellWidth || neighborY < 0 || neighborY >= CellHeight) continue;
					
					int32 neighborCellId = (neighborY * CellWidth) + neighborX;
					
					/*int32 startIndex = (*GridOffset)[neighborCellId];
					if (startIndex == -1) continue;*/
					
					int32 start = gridOffsets[neighborCellId];
					int32 end   = start + gridCounts[neighborCellId];
					//int32 count = gridCounts[neighborCellId];
					
					//for (int j = startIndex; j < *MemberActive; ++j)
					for(int32 j = start; j < end; ++j)
					{
						//if (members.CrowdsCellID[j] != neighborCellId) break;
						//if (i == j) continue;
						int32 otherAgent = gridMembers[start + j];
						if (i == otherAgent) continue;
						
						//FVector diff = location - members.CrowdsLocation[j];
						FVector diff = location - members.CrowdsLocation[otherAgent];
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
			UE_LOG(LogTemp, Log, TEXT("BOIDS SYSTEMS:: ACC %s"), *force.ToString());
		}
	}
};
