#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Generics/Data/FluxPrimeSpatialGridData.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeSpatialGridSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeSpatialGridDataConfig
{
    GENERATED_BODY()
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataTag;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Type Config")
    UScriptStruct* ExpectedStructType;
};

USTRUCT(BlueprintType)
struct FSpatialGridSystemDataTagConfig
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FFluxPrimeSpatialGridDataConfig EntityDataLocationConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FFluxPrimeSpatialGridDataConfig EntityDataCellIdConfig;

    bool IsValid() const
    {
        return EntityDataLocationConfig.EntityDataTag.IsValid() 
           && EntityDataCellIdConfig.EntityDataTag.IsValid();
    }
};

USTRUCT(BlueprintType)
struct FFluxPrimeSpatialGridSystem
{
    GENERATED_BODY()
    
private:
    UPROPERTY(EditAnywhere)
    FSpatialGridSystemDataTagConfig DataTagConfig;

    TArray<int16> EntityDataLocationID, EntityDataCellIdID;
    int32 TotalEntity = 0;
    
    UPROPERTY(EditAnywhere)
    float CellSize = 100.0f;

    UPROPERTY(EditAnywhere)
    FVector Origin = FVector(-10000.0f, -10000.0f, 0.0f); 

    UPROPERTY(EditAnywhere)
    int32 CellWidth = 200;
	
    UPROPERTY(EditAnywhere)
    int32 CellHeight = 200;
    
    int32 TotalCells = 100;
    TArray<int32> GridOffsets;
    TArray<int32> GridCounts;
    TArray<FFluxPrimeGridMemberData> GridMembers;
    bool IsAllDataValid = true;
    
private:
    void TryInitializedData(FFluxPrimeArchetypeDataStore& dataStore, FFluxPrimeSpatialGridDataConfig dataConfig, int16& dataID)
    {
        dataID = INDEX_NONE;
        FGameplayTag dataTag = dataConfig.EntityDataTag;
        UScriptStruct* dataType = dataConfig.ExpectedStructType;
		
        if (!IsAllDataValid) return;
        if (dataStore.TryResolveTag(dataTag, dataType, dataID)) return;

        IsAllDataValid = false;
        UE_LOG(LogFluxPrime, Error,
            TEXT("Data tag '%s' (expected struct '%s') was not found in this archetype's data store. ")
            TEXT("The owning system will not run. Add the tag and its struct to UFluxPrimeDataStoreIdentity."),
            *dataTag.ToString(),
            dataType ? *dataType->GetName() : TEXT("null"));
    }
    
public:
    void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
    {
        TotalCells = CellWidth * CellHeight;
        
        int32 countData = dataStores.Num();
        EntityDataLocationID.Init(0, countData);
        EntityDataCellIdID.Init(0, countData);
        
        GridCounts.Init(0, TotalCells);
        GridOffsets.Init(0, TotalCells);
        
        int32 indexDataId = 0;
        for (auto& dataStore : dataStores)
        {
            TryInitializedData(dataStore, DataTagConfig.EntityDataLocationConfig, EntityDataLocationID[indexDataId]);
            TryInitializedData(dataStore, DataTagConfig.EntityDataCellIdConfig, EntityDataCellIdID[indexDataId]);
            
            indexDataId++;
            TotalEntity += dataStore.GetTotalEntityCount();
        }
    }
	
    void UpdateSpatialGridSystem(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
    {
        TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_SpatialGrid_Systems);
        
        if (!IsAllDataValid)
        {
            UE_LOG(LogFluxPrime, Verbose, TEXT("SPATIAL GRID SYSTEM :: data invalid; system skipped"));
            return;
        }
        
        GridCounts.Init(0, TotalCells);
        GridOffsets.SetNumUninitialized(TotalCells);
        GridMembers.SetNumUninitialized(TotalEntity);
        
        int16 indexDataID = 0;
        for (auto& dataStore : dataStores)
        {
            FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
            FFluxPrimeWrapperInt32* entityDataCellId = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataCellIdID[indexDataID]));
            
            for (int i = 0; i < dataStore.GetActiveEntityCount(); ++i)
            {
                entityDataCellId[i].ValueInt32 = GetSpatialGridSystemsCellID(entityDataLocation[i]);
                GridCounts[entityDataCellId[i].ValueInt32]++;
            }
            
            indexDataID++;
        }
        
        int32 currentOffset = 0;
        for (int i = 0; i < TotalCells; ++i)
        {
            GridOffsets[i] = currentOffset;
            currentOffset += GridCounts[i];
        }

        TArray<int32> currentCellOffsets = GridOffsets; 
    
        indexDataID = 0;
        for (auto& dataStore : dataStores)
        {
            FFluxPrimeWrapperInt32* entityDataCellId = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataCellIdID[indexDataID]));

            for (int i = 0; i < dataStore.GetActiveEntityCount(); ++i)
            {
                int32 cellId = entityDataCellId[i].ValueInt32;
                int32 memberIndex = currentCellOffsets[cellId]++;
                
                GridMembers[memberIndex].EntityType = indexDataID;
                GridMembers[memberIndex].EntityIndex = i;
            }
            
            indexDataID++;
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
    
    TArray<FFluxPrimeGridMemberData>& GetGridMembers()
    {
        return GridMembers;
    }
};