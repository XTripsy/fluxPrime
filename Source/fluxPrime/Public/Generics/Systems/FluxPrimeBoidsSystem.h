#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "FluxPrimeSpatialGridSystem.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#if IF_WITH_EDITOR
#include "ProfilingDebugging/CpuProfilerTrace.h"
#endif
#include "FluxPrimeBoidsSystem.generated.h"

USTRUCT(BlueprintType)
struct FBoidsSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataAccelerationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataSizeTag;

	bool IsValid() const
	{
		return EntityDataLocationTag.IsValid() 
		   && EntityDataAccelerationTag.IsValid() 
		   && EntityDataSizeTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeBoidsSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FBoidsSystemDataTagConfig DataTagConfig;

	TArray<int16> EntityDataLocationID, EntityDataAccelerationID, EntityDataSizeID;
	
	UPROPERTY(EditAnywhere)
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
	
	UPROPERTY(EditAnywhere)
	TArray<FFluxPrimeSpatialGridSystem> SpatialGridSystems = TArray<FFluxPrimeSpatialGridSystem>();
	
	//FVector playerLocation;
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		int16 indexData = 0;
		for (auto& dataStore : dataStores)
		{
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataAccelerationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataAccelerationTag));
			EntityDataSizeID.Add(dataStore.GetDataID(DataTagConfig.EntityDataSizeTag));
		
			SpatialGridSystems[indexData].Initialized(dataStores);
			indexData++;
		}
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
	#if IF_WITH_EDITOR
	   TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Boids_Systems);
	#endif

	   for (int32 idx = 0; idx < dataStores.Num(); ++idx)
	   {
	      SpatialGridSystems[idx].UpdateSpatialGridSystem(dataStores[idx]);
	   }

	   for (int32 currentDataIdx = 0; currentDataIdx < dataStores.Num(); ++currentDataIdx)
	   {
	      auto& currentDataStore = dataStores[currentDataIdx];
	      
	      FVector* currentLocations = reinterpret_cast<FVector*>(currentDataStore.GetRawBufferData(EntityDataLocationID[currentDataIdx]));
	      FVector* currentAccelerations = reinterpret_cast<FVector*>(currentDataStore.GetRawBufferData(EntityDataAccelerationID[currentDataIdx]));
	      FFluxPrimeWrapperFloat* currentSizes = reinterpret_cast<FFluxPrimeWrapperFloat*>(currentDataStore.GetRawBufferData(EntityDataSizeID[currentDataIdx]));
	      
	      for (int32 i = 0; i < currentDataStore.GetActiveEntityCount(); ++i)
	      {
	         float separationRadius = FMath::Square(currentSizes[i].ValueFloat * 1.2f);
	         FVector tempLocation = currentLocations[i];
	         FVector force = FVector::Zero();
	         int32 overlapCount = 0;
	         
	         int32 agentCellX = FMath::FloorToInt((tempLocation.X - Origin.X) / CellSize);
	         int32 agentCellY = FMath::FloorToInt((tempLocation.Y - Origin.Y) / CellSize);
	         
	         for (int8 x = -1; x <= 1; ++x)
	         {
	            for (int8 y = -1; y <= 1; ++y)
	            {
	               int32 neighborX = agentCellX + x;
	               int32 neighborY = agentCellY + y;
	               
	               if (neighborX < 0 || neighborX >= CellWidth || neighborY < 0 || neighborY >= CellHeight) continue;
	               
	               int32 neighborCellId = (neighborY * CellWidth) + neighborX;
	               
	               for (int32 otherDataIdx = 0; otherDataIdx < SpatialGridSystems.Num(); ++otherDataIdx)
	               {
	                  TArray<int32>& gridOffsets = SpatialGridSystems[otherDataIdx].GetGridOffsets();
	                  TArray<int32>& gridCounts = SpatialGridSystems[otherDataIdx].GetGridCounts();
	                  TArray<int32>& gridMembers = SpatialGridSystems[otherDataIdx].GetGridMembers();
	                  
	                  int32 start = gridOffsets[neighborCellId];
	                  int32 end   = start + gridCounts[neighborCellId];
	                  
	                  FVector* otherLocations = reinterpret_cast<FVector*>(dataStores[otherDataIdx].GetRawBufferData(EntityDataLocationID[otherDataIdx]));
	                  
	                  for(int32 j = start; j < end; ++j)
	                  {
	                     int32 otherAgent = gridMembers[j];
	                     
	                     if (currentDataIdx == otherDataIdx && i == otherAgent) continue;
	                     
	                     FVector diff = tempLocation - otherLocations[otherAgent];
	                     diff.Z = 0;
	                     float distSq = diff.SizeSquared();
	                     
	                     if (distSq < separationRadius && distSq > 0.1f)
	                     {
	                        float dist = FMath::Sqrt(distSq);
	                        float pushFactor = 1.0f - (dist / (currentSizes[i].ValueFloat * 1.2f));
	                        
	                        force += (diff / dist) * pushFactor;
	                        overlapCount++;
	                     }
	                  }
	               }
	            }
	         }
	         
	         if (overlapCount > 0) force /= overlapCount;
	         force *= SeparationWeight;
	         currentAccelerations[i] += force;
	      }
	   }
	}
	
	/*void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
#if IF_WITH_EDITOR
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Boids_Systems);
#endif

		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			//playerLocation = UGameplayStatics::GetPlayerPawn(world, 0)->GetActorLocation();
		
			/*TArrayView<FVector> location = dataStore.GetMutableView<FVector>(LocationDataName);
			TArrayView<FVector> acceleration = dataStore.GetMutableView<FVector>(AccelerationDataName);
			TArrayView<int16> size = dataStore.GetMutableView<int16>(SizeDataName);#1#
			
			FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
			FVector* entityDataAcceleration = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataAccelerationID[indexDataID]));
			FFluxPrimeWrapperFloat* entityDataSize = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataSizeID[indexDataID]));
			
			SpatialGridSystems[indexDataID].UpdateSpatialGridSystem(dataStore);
			
			TArray<int32>& gridOffsets = SpatialGridSystems[indexDataID].GetGridOffsets();
			TArray<int32>& gridCounts = SpatialGridSystems[indexDataID].GetGridCounts();
			TArray<int32>& gridMembers = SpatialGridSystems[indexDataID].GetGridMembers();
			
			for (int32 i = 0; i < dataStore.GetActiveEntityCount(); ++i)
			{
				float separationRadius = FMath::Square(entityDataSize[i].ValueFloat * 1.2f);
				FVector tempLocation = entityDataLocation[i];
				FVector force = FVector::Zero();
				int32 overlapCount = 0;
				
				int32 agentCellX = FMath::FloorToInt((tempLocation.X - Origin.X) / CellSize);
				int32 agentCellY = FMath::FloorToInt((tempLocation.Y - Origin.Y) / CellSize);
				
				/*FVector diffToPlayer = location - playerLocation;
				diffToPlayer.Z = 0;
				float distToPlayerSq = diffToPlayer.SizeSquared();

				if (distToPlayerSq < separationRadius && distToPlayerSq > 0.1f)
				{
					float distToPlayer = FMath::Sqrt(distToPlayerSq);
					float pushFactor = 1.0f - (distToPlayer / separationRadius);
					FVector evasionForce = (diffToPlayer / distToPlayer) * pushFactor * separationRadius;
					members.CrowdsAcceleration[i] += evasionForce;
				}#1#
				
				for (int8 x = -1; x <= 1; ++x)
				{
					for (int8 y = -1; y <= 1; ++y)
					{
						int32 neighborX = agentCellX + x;
						int32 neighborY = agentCellY + y;
						
						if (neighborX < 0 || neighborX >= CellWidth || neighborY < 0 || neighborY >= CellHeight) continue;
						
						int32 neighborCellId = (neighborY * CellWidth) + neighborX;
						
						int32 start = gridOffsets[neighborCellId];
						int32 end   = start + gridCounts[neighborCellId];
						
						for(int32 j = start; j < end; ++j)
						{
							int32 otherAgent = gridMembers[start + j];
							if (i == otherAgent) continue;
							
							FVector diff = tempLocation - entityDataLocation[otherAgent];
							diff.Z = 0;
							float distSq = diff.SizeSquared();
							
							if (distSq < separationRadius && distSq > 0.1f)
							{
								float dist = FMath::Sqrt(distSq);
								float pushFactor = 1.0f - (dist / (entityDataSize[i].ValueFloat * 1.2f));
								
								force += (diff / dist) * pushFactor;
								overlapCount++;
							}
						}
					}
				}
				
				if (overlapCount > 0) force /= overlapCount;
				force *= SeparationWeight;
				entityDataAcceleration[i] += force;
			}
			
			indexDataID++;
		}
	}*/
	
	/*void ExecuteSystem(float deltaTime, FFluxPrimeDataStore& dataStore)
	{
#if IF_WITH_EDITOR
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Boids_Systems);
#endif
		
		//playerLocation = UGameplayStatics::GetPlayerPawn(world, 0)->GetActorLocation();
		
		/*TArrayView<FVector> location = dataStore.GetMutableView<FVector>(LocationDataName);
		TArrayView<FVector> acceleration = dataStore.GetMutableView<FVector>(AccelerationDataName);
		TArrayView<int16> size = dataStore.GetMutableView<int16>(SizeDataName);#1#
		
		FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID));
		FVector* entityDataAcceleration = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataAccelerationID));
		FFluxPrimeWrapperFloat* entityDataSize = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataSizeID));
		
		SpatialGridSystems.UpdateSpatialGridSystem(dataStore);
		
		TArray<int32>& gridOffsets = SpatialGridSystems.GetGridOffsets();
		TArray<int32>& gridCounts = SpatialGridSystems.GetGridCounts();
		TArray<int32>& gridMembers = SpatialGridSystems.GetGridMembers();
		
		for (int32 i = 0; i < dataStore.GetActiveEntityCount(); ++i)
		{
			float separationRadius = FMath::Square(entityDataSize[i].ValueFloat * 1.2f);
			FVector tempLocation = entityDataLocation[i];
			FVector force = FVector::Zero();
			int32 overlapCount = 0;
			
			int32 agentCellX = FMath::FloorToInt((tempLocation.X - Origin.X) / CellSize);
			int32 agentCellY = FMath::FloorToInt((tempLocation.Y - Origin.Y) / CellSize);
			
			/*FVector diffToPlayer = location - playerLocation;
			diffToPlayer.Z = 0;
			float distToPlayerSq = diffToPlayer.SizeSquared();

			if (distToPlayerSq < separationRadius && distToPlayerSq > 0.1f)
			{
				float distToPlayer = FMath::Sqrt(distToPlayerSq);
				float pushFactor = 1.0f - (distToPlayer / separationRadius);
				FVector evasionForce = (diffToPlayer / distToPlayer) * pushFactor * separationRadius;
				members.CrowdsAcceleration[i] += evasionForce;
			}#1#
			
			for (int8 x = -1; x <= 1; ++x)
			{
				for (int8 y = -1; y <= 1; ++y)
				{
					int32 neighborX = agentCellX + x;
					int32 neighborY = agentCellY + y;
					
					if (neighborX < 0 || neighborX >= CellWidth || neighborY < 0 || neighborY >= CellHeight) continue;
					
					int32 neighborCellId = (neighborY * CellWidth) + neighborX;
					
					int32 start = gridOffsets[neighborCellId];
					int32 end   = start + gridCounts[neighborCellId];
					
					for(int32 j = start; j < end; ++j)
					{
						int32 otherAgent = gridMembers[start + j];
						if (i == otherAgent) continue;
						
						FVector diff = tempLocation - entityDataLocation[otherAgent];
						diff.Z = 0;
						float distSq = diff.SizeSquared();
						
						if (distSq < separationRadius && distSq > 0.1f)
						{
							float dist = FMath::Sqrt(distSq);
							float pushFactor = 1.0f - (dist / (entityDataSize[i].ValueFloat * 1.2f));
							
							force += (diff / dist) * pushFactor;
							overlapCount++;
						}
					}
				}
			}
			
			if (overlapCount > 0) force /= overlapCount;
			force *= SeparationWeight;
			entityDataAcceleration[i] += force;
		}
	}*/
};
