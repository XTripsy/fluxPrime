#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystems.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeSpatialGridSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeSpatialGridSystemsContext
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere)
    TObjectPtr<UWorld> world;
    
    UPROPERTY(EditAnywhere)
    FVector origin = FVector(-10000.0f, -10000.0f, 0.0f); 
    
    UPROPERTY(EditAnywhere)
    float cellSize = 100.0f;

    UPROPERTY(EditAnywhere)
    int32 cellWidth = 200;
	
    UPROPERTY(EditAnywhere)
    int32 cellHeight = 200;

    UPROPERTY()
    bool isDebug = false;
    
    UPROPERTY()
    FColor debugColor = FColor::Black;
    
    //TStaticArray<FFluxPrimeCrowds, 2>* members = nullptr;
    FFluxPrimeCrowds* members = nullptr;
    /*TArray<int32>* gridOffset = nullptr;
    TArray<int32>* shortedIndex = nullptr; 
    int8* dataReadIndex = nullptr;*/ 
    uint16* memberActive = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeSpatialGridSystems : public FFluxPrimeBaseSystems
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
    int32 TotalCells = 100;

    UPROPERTY()
    bool IsDebug = false;
    
    UPROPERTY()
    FColor DebugColor = FColor::Black;
    
    //TStaticArray<FFluxPrimeCrowds, 2>* Members = nullptr;
    FFluxPrimeCrowds* Members = nullptr;
    /*TArray<int32>* GridOffset = nullptr;
    TArray<int32>* ShortedIndex = nullptr;
    int8* DataReadIndex = nullptr;*/ 
    uint16* MemberActive = nullptr;
    
    UPROPERTY()
    TArray<int32> GridOffsets;
    
    UPROPERTY()
    TArray<int32> GridCounts;
    
    UPROPERTY()
    TArray<int32> GridMembers;
private:
    void ShowDebug(FVector location, int32 id)
    {
        FVector textLocation = location + (FVector::UpVector * FluxConfig::DebugLocationSpatialGrid);
        FString debugData = FString::Printf(TEXT("Cell ID: %d"), id);
		
        DrawDebugString(
            World,
            textLocation,
            debugData,
            nullptr,
            FColor::Magenta,
            0.0f,
            false,
            FluxConfig::DebugScaleFont
        );
    }
    
    void DrawSpatialGridDebug()
    {
        if (!World) return;

        for (int32 x = 0; x <= CellWidth; x++)
        {
            FVector StartLocation = Origin + FVector(x * CellSize, 0, 1000);
            FVector EndLocation = StartLocation + FVector(0, CellHeight * CellSize, 1000);
        
            DrawDebugLine(World, StartLocation, EndLocation, DebugColor, true, 5, 0, 2.0f);
        }

        for (int32 y = 0; y <= CellHeight; y++)
        {
            FVector StartLocation = Origin + FVector(0, y * CellSize, 1000);
            FVector EndLocation = StartLocation + FVector(CellWidth * CellSize, 0, 1000);
        
            DrawDebugLine(World, StartLocation, EndLocation, DebugColor, true, 5, 0, 2.0f);
        }
    }
    
    int32 GetSpatialGridSystemsCellID(FVector location)
    {
        int32 CellX = FMath::FloorToInt((location.X - Origin.X) / CellSize);
        int32 CellY = FMath::FloorToInt((location.Y - Origin.Y) / CellSize);

        CellX = FMath::Clamp(CellX, 0, CellWidth - 1);
        CellY = FMath::Clamp(CellY, 0, CellHeight - 1);

        return (CellY * CellWidth) + CellX;
    }
	
public:
    void InitializedSpatialGridSystems(FFluxPrimeSpatialGridSystemsContext context)
    {
        check(context.world);
        check(context.members);
        /*check(context.gridOffset);
        check(context.dataReadIndex);*/
        
        World = context.world;
        IsDebug = context.isDebug;
        CellSize = context.cellSize;
        Origin = context.origin;
        CellWidth = context.cellWidth;
        CellHeight = context.cellHeight;
        DebugColor = context.debugColor;
        Members = context.members;
        /*GridOffset = context.gridOffset;
        ShortedIndex = context.shortedIndex;
        DataReadIndex = context.dataReadIndex;*/
        MemberActive = context.memberActive;
    }
	
    void BakeSpatialGridSystems()
    {
        check(World);
        
        TotalCells = CellWidth * CellHeight;
        
        //int32 totalMember = (*Members)[0].CrowdsID.Num();
        int32 totalMember = Members->CrowdsID.Num();
        GridCounts.Init(0, TotalCells);
        GridOffsets.SetNum(TotalCells);
        GridMembers.SetNum(totalMember);
        
        if (IsDebug) DrawSpatialGridDebug();
    }
    
    void UpdateSpatialGridSystem()
    {
        //auto& dataReadIndex = *DataReadIndex;
        auto& members = *Members;
        auto& activeMembers = *MemberActive;
        /*auto& shortedIndex = *ShortedIndex;
        auto& gridOffset = *GridOffset;*/
        
        int32 totalMember = members.CrowdsID.Num();
        GridCounts.Init(0, TotalCells);
        GridOffsets.SetNumUninitialized(TotalCells);
        GridMembers.SetNumUninitialized(totalMember);
        
        /*int8 writeIndex = (dataReadIndex + 1) % 2;
        FFluxPrimeCrowds& readBuffer = members[dataReadIndex];
        FFluxPrimeCrowds& writeBuffer = members[writeIndex];*/

        int32 offsets = 0;
        for (int i = 0; i < TotalCells; ++i)
        {
            GridOffsets[i] = offsets;
            offsets += GridCounts[i];
        }
        
        TArray<int32> memberOffsets = GridOffsets;
        for (int i = 0; i < activeMembers; ++i)
        {
            /*FVector location = readBuffer.CrowdsLocation[i];
            readBuffer.CrowdsCellID[i] = GetSpatialGridSystemsCellID(location);
            int32 cellId = readBuffer.CrowdsCellID[i];*/
            FVector location = members.CrowdsLocation[i];
            members.CrowdsCellID[i] = GetSpatialGridSystemsCellID(location);
            int32 cellId = members.CrowdsCellID[i];
            
            GridCounts[cellId]++;
            
            int32 memberIndex = memberOffsets[cellId]++;
            GridMembers[memberIndex] = i;
            
            if (IsDebug) ShowDebug(location, cellId);
        }
        
        /*shortedIndex.SetNumUninitialized(activeMembers, EAllowShrinking::No);
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
            writeBuffer.CrowdsState[i] = readBuffer.CrowdsState[tempShortedIndex];
            writeBuffer.CrowdsDamage[i] = readBuffer.CrowdsDamage[tempShortedIndex];
            writeBuffer.CrowdsState[i] = readBuffer.CrowdsState[tempShortedIndex];
            writeBuffer.CrowdsTargetLocation[i] = readBuffer.CrowdsTargetLocation[tempShortedIndex];
            writeBuffer.CrowdsIndexNavigationPath[i] = readBuffer.CrowdsIndexNavigationPath[tempShortedIndex];
            writeBuffer.CrowdsTotalNavigationPath[i] = readBuffer.CrowdsTotalNavigationPath[tempShortedIndex];
            writeBuffer.CrowdsNavigationPath[i] = readBuffer.CrowdsNavigationPath[tempShortedIndex];
            //writeBuffer.CrowdsCurrentTargetLocationPath[i] = readBuffer.CrowdsCurrentTargetLocationPath[tempShortedIndex];
            writeBuffer.CrowdsRequestNavigationPath[i] = readBuffer.CrowdsRequestNavigationPath[tempShortedIndex];
            writeBuffer.CrowdsAnimationMapping[i] = readBuffer.CrowdsAnimationMapping[tempShortedIndex];
            writeBuffer.CrowdsAnimationState[i] = readBuffer.CrowdsAnimationState[tempShortedIndex];
            writeBuffer.CrowdsStartTimeAnimation[i] = readBuffer.CrowdsStartTimeAnimation[tempShortedIndex];
            writeBuffer.CrowdsPreviousAnimationFrame[i] = readBuffer.CrowdsPreviousAnimationFrame[tempShortedIndex];
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
        
        dataReadIndex = writeIndex;*/
    }
    
    TArray<int32>& GetGridOffsets()
    {
        return GridOffsets;
    }
    
    TArray<int32>& GetGridCounts()
    {
        return GridCounts;
    }
    
    TArray<int32>& GetGridMembers()
    {
        return GridMembers;
    }
};