#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeSpatialGridSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeSpatialGridSystems
{
    GENERATED_BODY()
    
private:
    UPROPERTY(VisibleAnywhere)
    float CellSize = 0;
    
    UPROPERTY(VisibleAnywhere)
    int32 TotalCells = 0;
    
public:
    void InitializeSpatialGridSystem(float cellSize, int32 totalCells)
    {
        CellSize = cellSize;
        TotalCells = totalCells;
    }
    
    void UpdateSpatialGridSystem(FFluxPrimeCrowds members[2], TArray<int32>& gridOffset, TArray<int32>& shortedIndex, int8& dataReadIndex, int32 activeMembers)
    {
        int8 WriteIndex = (dataReadIndex + 1) % 2;
        FFluxPrimeCrowds& ReadBuffer = members[dataReadIndex];
        FFluxPrimeCrowds& WriteBuffer = members[WriteIndex];
        
        for (int i = 0; i < activeMembers; ++i)
        {
            FVector location = ReadBuffer.CrowdsLocation[i];
            
            int32 gridX = FMath::FloorToInt(location.X / CellSize);
            int32 gridY = FMath::FloorToInt(location.Y / CellSize);
            
            int32 hash = (gridX * 73856093) ^ (gridY * 193496629);
            
            ReadBuffer.CrowdsCellID[i] = FMath::Abs(hash) % TotalCells;
        }
        
        shortedIndex.SetNumUninitialized(activeMembers, EAllowShrinking::No);
        for (int i = 0; i < activeMembers; ++i)
        {
            shortedIndex[i] = i;
        }
        
        Algo::Sort(shortedIndex, [&ReadBuffer](int32 a, int32 b)
            {
                return ReadBuffer.CrowdsCellID[a] < ReadBuffer.CrowdsCellID[b];
            }
        );
        
        for (int i = 0; i < activeMembers; ++i)
        {
            int32 tempShortedIndex = shortedIndex[i];
            
            WriteBuffer.CrowdsLocation[i] = ReadBuffer.CrowdsLocation[tempShortedIndex];
            WriteBuffer.CrowdsRotation[i] = ReadBuffer.CrowdsRotation[tempShortedIndex];
            WriteBuffer.CrowdsAcceleration[i] = ReadBuffer.CrowdsAcceleration[tempShortedIndex];
            WriteBuffer.CrowdsVelocity[i] = ReadBuffer.CrowdsVelocity[tempShortedIndex];
            WriteBuffer.CrowdsID[i] = ReadBuffer.CrowdsID[tempShortedIndex];
            WriteBuffer.CrowdsCellID[i] = ReadBuffer.CrowdsCellID[tempShortedIndex];
            WriteBuffer.CrowdsMaxSpeed[i] = ReadBuffer.CrowdsMaxSpeed[tempShortedIndex];
            WriteBuffer.CrowdsType[i] = ReadBuffer.CrowdsType[tempShortedIndex];
            WriteBuffer.CrowdsHealth[i] = ReadBuffer.CrowdsHealth[tempShortedIndex];
            WriteBuffer.CrowdsSize[i] = ReadBuffer.CrowdsSize[tempShortedIndex];
            WriteBuffer.CrowdsDamage[i] = ReadBuffer.CrowdsDamage[tempShortedIndex];
            WriteBuffer.CrowdsTargetLocation[i] = ReadBuffer.CrowdsTargetLocation[tempShortedIndex];
            WriteBuffer.CrowdsIndexNavigationPath[i] = ReadBuffer.CrowdsIndexNavigationPath[tempShortedIndex];
            WriteBuffer.CrowdsTotalNavigationPath[i] = ReadBuffer.CrowdsTotalNavigationPath[tempShortedIndex];
            WriteBuffer.CrowdsNavigationPath[i] = ReadBuffer.CrowdsNavigationPath[tempShortedIndex];
            WriteBuffer.CrowdsAnimationData[i] = ReadBuffer.CrowdsAnimationData[tempShortedIndex];
            WriteBuffer.CrowdsAnimationIndex[i] = ReadBuffer.CrowdsAnimationIndex[tempShortedIndex];
            WriteBuffer.CrowdsStartTimeAnimationFrame[i] = ReadBuffer.CrowdsStartTimeAnimationFrame[tempShortedIndex];
        }
        
        gridOffset.Init(-1, TotalCells);
        
        gridOffset[WriteBuffer.CrowdsCellID[0]] = 0;

        for (int i = 1; i < activeMembers; ++i)
        {
            if (WriteBuffer.CrowdsCellID[i] != WriteBuffer.CrowdsCellID[i - 1])
            {
                gridOffset[WriteBuffer.CrowdsCellID[i]] = i;
            }
        }
        
        dataReadIndex = WriteIndex;
    }
};