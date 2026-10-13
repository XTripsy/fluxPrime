#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeSpatialGridSystem.h"
#include "Generics/Data/FluxPrimePayloadData.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperBool.h"
#include "Domains/Crowds/Event/FluxPrimeCrowdsEvent.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "FluxPrimeSnapshotSystem.generated.h"

USTRUCT(BlueprintType)
struct FSnapshotSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataIdTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataCellIdTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataStateTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLocationTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataCurrentWaypointLocationTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataTargetLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataConditionAliveTag;
	
	bool IsValid() const
	{
		return EntityDataIdTag.IsValid() 
		   && EntityDataCellIdTag.IsValid()
		   && EntityDataStateTag.IsValid()
		   && EntityDataLocationTag.IsValid()
		   && EntityDataCurrentWaypointLocationTag.IsValid()
		   && EntityDataTargetLocationTag.IsValid()
		   && EntityDataConditionAliveTag.IsValid();
	}
};

USTRUCT()
struct FFluxPrimeSnapshotSystemContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world = nullptr;
	
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
	FOnCrowdsManagerActionChange* onManagerActionChange = nullptr;
	bool hasAuthority = false;
};

USTRUCT(BlueprintType)
struct FFluxPrimeSnapshotSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FSnapshotSystemDataTagConfig DataTagConfig;
	
	UPROPERTY(EditAnywhere)
	FFluxPrimeSpatialGridSystem SpatialGridSystems = FFluxPrimeSpatialGridSystem();
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "1s is equal 1Hz, so its mean 0.1s is equal 10Hz"))
	float CycleFrequencyBuffer = 0.1f;
	
	UPROPERTY(EditAnywhere)
	int32 MaxBandwidthBuffer = 128;
	
	FOnCrowdsManagerActionChange* OnManagerActionChange = nullptr;
	TArray<int16> EntityDataIdID, EntityDataCellIdID, EntityDataStateID, EntityDataLocationID, EntityDataCurrentWaypointLocationID, EntityDataTargetLocationID, EntityDataConditionAliveID;
	TArray<int32> NeighborCellID;
	
	FFluxPrimeSnapshotDataPayload SnapshotDataPayload;
	
	float CurrentTime = 0;
	int32 NetworkTickCounter = 0;
	bool HasAuthority = false;
	
private:
	void UpdateNeighborCellID(const float cellSize, const float cellWidth, const float cellHeight, 
		const FVector origin, const FVector targetLocation)
	{
		int32 indexNeighborCellId = 0;
		
		int32 agentCellX = FMath::FloorToInt((targetLocation.X - origin.X) / cellSize);
		int32 agentCellY = FMath::FloorToInt((targetLocation.Y - origin.Y) / cellSize);
		
		agentCellX = FMath::Clamp(agentCellX, 0, cellWidth - 1);
		agentCellY = FMath::Clamp(agentCellY, 0, cellHeight - 1);
		
		for (int x = -1; x <= 1; ++x)
		{
			for (int y = -1; y <= 1; ++y)
			{
				int32 neighborX = agentCellX + x;
				int32 neighborY = agentCellY + y;
				
				int32 neighborCellId = (neighborY * cellWidth) + neighborX;
				NeighborCellID[indexNeighborCellId] = neighborCellId;
				indexNeighborCellId++;
			}
		}
	}
	
public:
	void Initialized(FFluxPrimeSnapshotSystemContext context)
	{
		check(context.world);
		check(context.onManagerActionChange);
		
		OnManagerActionChange = context.onManagerActionChange;
		HasAuthority = context.hasAuthority;
		
		NeighborCellID.Init(0, 9);
		
		int16 indexData = 0;
		for (auto& dataStore : *context.dataStores)
		{
			SpatialGridSystems.Initialized(*context.dataStores);
			
			EntityDataIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataIdTag));
			EntityDataCellIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCellIdTag));
			EntityDataStateID.Add(dataStore.GetDataID(DataTagConfig.EntityDataStateTag));
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataCurrentWaypointLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCurrentWaypointLocationTag));
			EntityDataTargetLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataTargetLocationTag));
			EntityDataConditionAliveID.Add(dataStore.GetDataID(DataTagConfig.EntityDataConditionAliveTag));
		}
		
		SnapshotDataPayload.SnapshotDatas.Init(FFluxPrimeSnapshotData(), MaxBandwidthBuffer);
	}
	
	void SyncData(TArray<FFluxPrimeArchetypeDataStore>& dataStores, const TArray<FFluxPrimeSnapshotData>& dataSnapshots)
	{
		if (dataStores.IsEmpty() || dataSnapshots.IsEmpty()) return;
		
		for (auto& pair : dataSnapshots)
		{
			if (pair.CrowdType < 0 || pair.CrowdIndex < 0) continue;
			
			FVector* entityDataCurrentWaypointLocation = reinterpret_cast<FVector*>(dataStores[pair.CrowdType].GetRawBufferData(EntityDataCurrentWaypointLocationID[pair.CrowdType]));
			FFluxPrimeWrapperInt8* entityDataState = reinterpret_cast<FFluxPrimeWrapperInt8*>(dataStores[pair.CrowdType].GetRawBufferData(EntityDataStateID[pair.CrowdType]));
			FFluxPrimeWrapperBool* entityDataConditionAlive = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStores[pair.CrowdType].GetRawBufferData(EntityDataConditionAliveID[pair.CrowdType]));
			
			check(entityDataCurrentWaypointLocation);
			check(entityDataState);
			check(entityDataConditionAlive);
			
			entityDataCurrentWaypointLocation[pair.CrowdIndex] = pair.CrowdNewWaypointLocation;
			entityDataState[pair.CrowdIndex].ValueInt8 = pair.CrowdState;
			entityDataConditionAlive[pair.CrowdIndex].ValueBool = pair.CrowdConditionAlive;
		}
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		if (!HasAuthority) return;
		
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Snapshot_Systems);
		
		check(OnManagerActionChange);
		
		CurrentTime += deltaTime;
    
		if (CurrentTime < CycleFrequencyBuffer) return;
		
		SpatialGridSystems.UpdateSpatialGridSystem(dataStores);
		
		CurrentTime = 0.0f;
		NetworkTickCounter++;
		
		auto& snapshotData = SnapshotDataPayload.SnapshotDatas;
		
		int16 indexDataID = 0;
		bool bSendData = false;
		int32 indexEntityData = INDEX_NONE;
		
		const float cellSize = SpatialGridSystems.GetCellSize();
		const float cellWidth = SpatialGridSystems.GetCellWidth();
		const float cellHeight = SpatialGridSystems.GetCellHeight();
		const FVector origin = SpatialGridSystems.GetOrigin();
		
		for (auto& dataStore : dataStores)
		{
			FFluxPrimeWrapperInt16* entityDataId = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataIdID[indexDataID]));
			FFluxPrimeWrapperInt32* entityDataCellId = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataCellIdID[indexDataID]));
			FFluxPrimeWrapperInt8* entityDataState = reinterpret_cast<FFluxPrimeWrapperInt8*>(dataStore.GetRawBufferData(EntityDataStateID[indexDataID]));
			FFluxPrimeWrapperBool* entityDataConditionAlive = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataConditionAliveID[indexDataID]));
			FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
			FVector* entityDataTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]));
			
			check(entityDataId);
			check(entityDataCellId);
			check(entityDataState);
			check(entityDataConditionAlive);
			check(entityDataLocation);
			check(entityDataTargetLocation);
			
			for (int i = 0; i < dataStore.GetTotalEntityCount(); ++i)
			{
				UpdateNeighborCellID(cellSize, cellWidth, cellHeight, origin, entityDataTargetLocation[i]);
				
				bSendData = false;
				int32 entityCellID = entityDataCellId[i].ValueInt32;
				
				if (NeighborCellID.Contains(entityCellID))
				{
					float distSq = FVector::DistSquared(entityDataLocation[i], entityDataTargetLocation[i]);
        
					if (distSq < 4000000.0f) bSendData = true;
					else if (distSq < 25000000.0f) bSendData = (NetworkTickCounter % 2 == 0);
					else bSendData = (NetworkTickCounter % 5 == 0);
				}
				else if (NetworkTickCounter % 20 == 0) bSendData = true;
				
				if (!bSendData) continue;
				
				indexEntityData++;
				snapshotData[indexEntityData].CrowdIndex = i;
				snapshotData[indexEntityData].CrowdType = indexDataID;
				snapshotData[indexEntityData].CrowdState = entityDataState[i].ValueInt8;
				snapshotData[indexEntityData].CrowdConditionAlive = entityDataConditionAlive[i].ValueBool;
				snapshotData[indexEntityData].CrowdNewWaypointLocation = entityDataLocation[i];
				
				if (indexEntityData >= MaxBandwidthBuffer-1)
				{
					FFluxPrimeSnapshotDataPayload data;
					data.SnapshotDatas.Append(snapshotData.GetData(), indexEntityData + 1);
					FInstancedStruct payload = FInstancedStruct::Make(data);
		
					OnManagerActionChange->ExecuteIfBound(payload);
					
					indexEntityData = INDEX_NONE;
				}
			}
			
			indexDataID++;
		}
		
		if (indexEntityData > INDEX_NONE)
		{
			FFluxPrimeSnapshotDataPayload data;
			data.SnapshotDatas.Append(snapshotData.GetData(), indexEntityData + 1);
			FInstancedStruct payload = FInstancedStruct::Make(data);
		
			OnManagerActionChange->ExecuteIfBound(payload);
		}
	}
};
