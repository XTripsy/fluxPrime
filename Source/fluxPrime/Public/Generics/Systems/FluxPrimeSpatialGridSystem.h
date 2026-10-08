#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeSpatialGridSystem.generated.h"

USTRUCT(BlueprintType)
struct FSpatialGridSystemDataTagConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataLocationTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataCellIdTag;

    bool IsValid() const
    {
        return EntityDataLocationTag.IsValid() 
           && EntityDataCellIdTag.IsValid();
    }
};

USTRUCT(BlueprintType)
struct FFluxPrimeSpatialGridSystem : public FFluxPrimeBaseSystem
{
    GENERATED_BODY()
    
private:
    UPROPERTY(EditAnywhere)
    FSpatialGridSystemDataTagConfig DataTagConfig;

    TArray<int16> EntityDataLocationID, EntityDataCellIdID;
    
    UPROPERTY(EditAnywhere)
    float CellSize = 100.0f;

    UPROPERTY(EditAnywhere)
    FVector Origin = FVector(-10000.0f, -10000.0f, 0.0f); 

    UPROPERTY(EditAnywhere)
    int32 CellWidth = 200;
	
    UPROPERTY(EditAnywhere)
    int32 CellHeight = 200;
    
    UPROPERTY()
    int32 TotalCells = 100;
    
    UPROPERTY()
    TArray<int32> GridOffsets;
    
    UPROPERTY()
    TArray<int32> GridCounts;
    
    UPROPERTY()
    TArray<int32> GridMembers;
    
public:
    void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
    {
        TotalCells = CellWidth * CellHeight;
        
        GridCounts.Init(0, TotalCells);
        GridOffsets.Init(0, TotalCells);
        
        for (auto& dataStore : dataStores)
        {
            EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
            EntityDataCellIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCellIdTag));
        }
    }
	
    //void UpdateSpatialGridSystem(const FFluxPrimeArchetypeDataStore& dataStore, const int16 indexDataID)
    void UpdateSpatialGridSystem(const FFluxPrimeArchetypeDataStore& dataStore)
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_SpatialGrid_Systems);
        
        int16 indexDataID = 0;
        
        FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
        FFluxPrimeWrapperInt32* entityDataCellId = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataCellIdID[indexDataID]));

        GridCounts.Init(0, TotalCells);
        GridOffsets.SetNumUninitialized(TotalCells);
        GridMembers.SetNumUninitialized(dataStore.GetTotalEntityCount());
        
        int32 offsets = 0;
        for (int i = 0; i < TotalCells; ++i)
        {
            GridOffsets[i] = offsets;
            offsets += GridCounts[i];
        }
        
        TArray<int32> memberOffsets = GridOffsets;
        for (int i = 0; i < dataStore.GetActiveEntityCount(); ++i)
        {
            entityDataCellId[i].ValueInt32 = GetSpatialGridSystemsCellID(entityDataLocation[i]);
            int32 cellId = entityDataCellId[i].ValueInt32;
            
            GridCounts[cellId]++;
            
            int32 memberIndex = memberOffsets[cellId]++;
            GridMembers[memberIndex] = i;
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
    
    FVector GetOrigin() const
    {
        return Origin;
    }
    
    int32 GetTotalCells() const
    {
        return TotalCells;
    }
    
    float GetCellSize() const
    {
        return CellSize;
    }
    
    float GetCellWidth() const
    {
        return CellWidth;
    }
    
    float GetCellHeight() const
    {
        return CellHeight;
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