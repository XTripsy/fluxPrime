#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystems.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeSpatialGridSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeSpatialGridSystems : public FFluxPrimeBaseSystems
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
    int32 TotalCells = 100;

    UPROPERTY(EditAnywhere)
    TArray<int32> CellID;
    
    UPROPERTY()
    bool IsDebug = false;
    
private:
    void ShowDebug(TObjectPtr<UWorld> world, FVector location, int32 id)
    {
        FVector textLocation = location + (FVector::UpVector * FluxConfig::DebugLocationSpatialGrid);
        FString debugData = FString::Printf(TEXT("Cell ID: %d"), id);
		
        DrawDebugString(
            world,
            textLocation,
            debugData,
            nullptr,
            FColor::Magenta,
            0.0f,
            false,
            FluxConfig::DebugScaleFont
        );
    }
    
    void DrawSpatialGridDebug(UWorld* World)
    {
        if (!World) return;

        for (int32 x = 0; x <= CellWidth; x++)
        {
            FVector StartLocation = Origin + FVector(x * CellSize, 0, 1000);
            FVector EndLocation = StartLocation + FVector(0, CellHeight * CellSize, 1000);
        
            DrawDebugLine(World, StartLocation, EndLocation, FColor::Green, true, 5, 0, 2.0f);
        }

        for (int32 y = 0; y <= CellHeight; y++)
        {
            FVector StartLocation = Origin + FVector(0, y * CellSize, 1000);
            FVector EndLocation = StartLocation + FVector(CellWidth * CellSize, 0, 1000);
        
            DrawDebugLine(World, StartLocation, EndLocation, FColor::Green, true, 5, 0, 2.0f);
        }
    }
	
public:
    void InitializedSpatialGridSystems(bool isDebug, float cellSize, FVector origin, int32 cellWidth, int32 cellHeight)
    {
        IsDebug = isDebug;
        CellSize = cellSize;
        Origin = origin;
        CellWidth = cellWidth;
        CellHeight = cellHeight;
    }
	
    void BakeSpatialGridSystems(TObjectPtr<UWorld> world)
    {
        TotalCells = CellWidth * CellHeight;
        CellID.Init(0, TotalCells);

        for (int32 y = 0; y < CellHeight; y++)
        {
            for (int32 x = 0; x < CellWidth; x++)
            {
                int32 IndexID = (y * CellWidth) + x;

                CellID[IndexID] = IndexID;
            }
        }
        
        if (IsDebug) DrawSpatialGridDebug(world);
    }
    
    int32 GetSpatialGridSystemsCellID(FVector location)
    {
        int32 CellX = FMath::FloorToInt((location.X - Origin.X) / CellSize);
        int32 CellY = FMath::FloorToInt((location.Y - Origin.Y) / CellSize);

        CellX = FMath::Clamp(CellX, 0, CellWidth - 1);
        CellY = FMath::Clamp(CellY, 0, CellHeight - 1);

        int32 IndexID = (CellY * CellWidth) + CellX;

        return CellID[IndexID];
    }
    
    void UpdateSpatialGridSystem(TObjectPtr<UWorld> world, TStaticArray<FFluxPrimeCrowds, 2>& members, TArray<int32>& gridOffset, TArray<int32>& shortedIndex, int8& dataReadIndex, int32 activeMembers)
    {
        int8 writeIndex = (dataReadIndex + 1) % 2;
        FFluxPrimeCrowds& readBuffer = members[dataReadIndex];
        FFluxPrimeCrowds& writeBuffer = members[writeIndex];
        
        for (int i = 0; i < activeMembers; ++i)
        {
            FVector location = readBuffer.CrowdsLocation[i];
            
            readBuffer.CrowdsCellID[i] = GetSpatialGridSystemsCellID(location);
            
            if (IsDebug) ShowDebug(world, location, readBuffer.CrowdsCellID[i]);
        }
        
        shortedIndex.SetNumUninitialized(activeMembers, EAllowShrinking::No);
        for (int i = 0; i < activeMembers; ++i)
        {
            shortedIndex[i] = i;
        }
        
        Algo::Sort(shortedIndex, [&readBuffer](int32 a, int32 b)
            {
                return readBuffer.CrowdsCellID[a] < readBuffer.CrowdsCellID[b];
            }
        );
        
        for (int i = 0; i < activeMembers; ++i)
        {
            int32 tempShortedIndex = shortedIndex[i];
            
            writeBuffer.CrowdsLocation[i] = readBuffer.CrowdsLocation[tempShortedIndex];
            writeBuffer.CrowdsRotation[i] = readBuffer.CrowdsRotation[tempShortedIndex];
            writeBuffer.CrowdsAcceleration[i] = readBuffer.CrowdsAcceleration[tempShortedIndex];
            writeBuffer.CrowdsVelocity[i] = readBuffer.CrowdsVelocity[tempShortedIndex];
            writeBuffer.CrowdsID[i] = readBuffer.CrowdsID[tempShortedIndex];
            writeBuffer.CrowdsCellID[i] = readBuffer.CrowdsCellID[tempShortedIndex];
            writeBuffer.CrowdsMaxSpeed[i] = readBuffer.CrowdsMaxSpeed[tempShortedIndex];
            writeBuffer.CrowdsType[i] = readBuffer.CrowdsType[tempShortedIndex];
            writeBuffer.CrowdsHealth[i] = readBuffer.CrowdsHealth[tempShortedIndex];
            writeBuffer.CrowdsSize[i] = readBuffer.CrowdsSize[tempShortedIndex];
            writeBuffer.CrowdsDamage[i] = readBuffer.CrowdsDamage[tempShortedIndex];
            writeBuffer.CrowdsTargetLocation[i] = readBuffer.CrowdsTargetLocation[tempShortedIndex];
            writeBuffer.CrowdsIndexNavigationPath[i] = readBuffer.CrowdsIndexNavigationPath[tempShortedIndex];
            writeBuffer.CrowdsTotalNavigationPath[i] = readBuffer.CrowdsTotalNavigationPath[tempShortedIndex];
            writeBuffer.CrowdsNavigationPath[i] = readBuffer.CrowdsNavigationPath[tempShortedIndex];
            writeBuffer.CrowdsAnimationData[i] = readBuffer.CrowdsAnimationData[tempShortedIndex];
            writeBuffer.CrowdsAnimationIndex[i] = readBuffer.CrowdsAnimationIndex[tempShortedIndex];
            writeBuffer.CrowdsStartTimeAnimationFrame[i] = readBuffer.CrowdsStartTimeAnimationFrame[tempShortedIndex];
            writeBuffer.CrowdsCurrentAnimationFrame[i] = readBuffer.CrowdsCurrentAnimationFrame[tempShortedIndex];
        }
        
        gridOffset.Init(-1, TotalCells);
        
        gridOffset[writeBuffer.CrowdsCellID[0]] = 0;

        for (int i = 1; i < activeMembers; ++i)
        {
            if (writeBuffer.CrowdsCellID[i] != writeBuffer.CrowdsCellID[i - 1])
            {
                gridOffset[writeBuffer.CrowdsCellID[i]] = i;
            }
        }
        
        dataReadIndex = writeIndex;
    }
};