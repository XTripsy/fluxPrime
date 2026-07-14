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
    //FFluxPrimeCrowds* members = nullptr;
    TArray<FVector>* locationCrowds = nullptr;
    TArray<int32>* cellIDCrowds = nullptr;
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
    //FFluxPrimeCrowds* Members = nullptr;
    TArray<FVector>* LocationCrowds = nullptr;
    TArray<int32>* CellIDCrowds = nullptr;
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
    
public:
    void InitializedSpatialGridSystems(FFluxPrimeSpatialGridSystemsContext context)
    {
        check(context.world);
        check(context.locationCrowds);
        check(context.cellIDCrowds);
        
        World = context.world;
        IsDebug = context.isDebug;
        CellSize = context.cellSize;
        Origin = context.origin;
        CellWidth = context.cellWidth;
        CellHeight = context.cellHeight;
        DebugColor = context.debugColor;
        LocationCrowds = context.locationCrowds;
        CellIDCrowds = context.cellIDCrowds;
        MemberActive = context.memberActive;
    }
	
    void BakeSpatialGridSystems()
    {
        check(World);
        
        TotalCells = CellWidth * CellHeight;
        
        auto& cellID = *CellIDCrowds;
        int32 totalMember = cellID.Num();
        GridCounts.Init(0, TotalCells);
        GridOffsets.SetNum(TotalCells);
        GridMembers.SetNum(totalMember);
        
        if (IsDebug) DrawSpatialGridDebug();
    }
    
    void UpdateSpatialGridSystem()
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_SpatialGrid_Systems);
        
        auto& activeMembers = *MemberActive;
        
        auto& locationCrowds = *LocationCrowds;
        auto& cellID = *CellIDCrowds;
        
        int32 totalMember = cellID.Num();
        GridCounts.Init(0, TotalCells);
        GridOffsets.SetNumUninitialized(TotalCells);
        GridMembers.SetNumUninitialized(totalMember);

        int32 offsets = 0;
        for (int i = 0; i < TotalCells; ++i)
        {
            GridOffsets[i] = offsets;
            offsets += GridCounts[i];
        }
        
        TArray<int32> memberOffsets = GridOffsets;
        for (int i = 0; i < activeMembers; ++i)
        {
            FVector location = locationCrowds[i];
            cellID[i] = GetSpatialGridSystemsCellID(location);
            int32 cellId = cellID[i];
            
            GridCounts[cellId]++;
            
            int32 memberIndex = memberOffsets[cellId]++;
            GridMembers[memberIndex] = i;
            
            if (IsDebug) ShowDebug(location, cellId);
        }
    }
    
    int32 GetSpatialGridSystemsCellID(FVector location) const
    {
        int32 CellX = FMath::FloorToInt((location.X - Origin.X) / CellSize);
        int32 CellY = FMath::FloorToInt((location.Y - Origin.Y) / CellSize);

        CellX = FMath::Clamp(CellX, 0, CellWidth - 1);
        CellY = FMath::Clamp(CellY, 0, CellHeight - 1);

        return (CellY * CellWidth) + CellX;
    }
    
    int32 GetTotalCells() const
    {
        return TotalCells;
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