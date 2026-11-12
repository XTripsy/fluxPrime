#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "FluxPrimeVisualInterpolationSystem.generated.h"

USTRUCT(BlueprintType)
struct FVisualInterpolationSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLocationConfig;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRotationConfig;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataCurrentWaypointLocationConfig;

	bool IsValid() const
	{
		return EntityDataLocationConfig.EntityDataTag.IsValid()
		   && EntityDataRotationConfig.EntityDataTag.IsValid()
		   && EntityDataCurrentWaypointLocationConfig.EntityDataTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeVisualInterpolationSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FVisualInterpolationSystemDataTagConfig DataTagConfig;
	
	TArray<int16> EntityDataLocationID, EntityDataRotationID, EntityDataCurrentWaypointLocationID;
	
private:
	void VisualInterpolation(float deltaTime, FVector& currentLocation, FRotator& currentRotation, FVector targetLocation)
	{
		FVector direction = targetLocation - currentLocation;
		direction.Z = 0.0f; 

		if (!direction.IsNearlyZero())
		{
			float targetYaw = direction.Rotation().Yaw - 85.0f;
			FRotator targetRotation = currentRotation;
			targetRotation.Yaw = targetYaw;
			currentRotation = FMath::RInterpTo(currentRotation, targetRotation, deltaTime, 10.0f);
		}

		currentLocation = FMath::VInterpTo(currentLocation, targetLocation, deltaTime, 10.0f);
	}
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		int32 countData = dataStores.Num();
		EntityDataLocationID.Init(0, countData);
		EntityDataRotationID.Init(0, countData);
		EntityDataCurrentWaypointLocationID.Init(0, countData);
		
		int32 indexDataId = 0;
		for (auto& dataStore : dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataLocationConfig, EntityDataLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRotationConfig, EntityDataRotationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataCurrentWaypointLocationConfig, EntityDataCurrentWaypointLocationID[indexDataId]);
			indexDataId = 0;
		}
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_VisualInterpolation_Systems);
		
		if (!IsAllDataValid)
		{
			UE_LOG(LogTemp, Error, TEXT("VISUAL INTERPOLATION SYSTEM :: SOME DATA NOT VALID"));
			return;
		}
		
		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
			FRotator* entityDataRotation = reinterpret_cast<FRotator*>(dataStore.GetRawBufferData(EntityDataRotationID[indexDataID]));
			FVector* entityDataCurrentWaypointLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointLocationID[indexDataID]));
			
			check(entityDataLocation);
			check(entityDataRotation); 
			check(entityDataCurrentWaypointLocation);
			
			for (int i = 0; i < dataStore.GetActiveEntityCount(); ++i)
			{
				VisualInterpolation(deltaTime, entityDataLocation[i], entityDataRotation[i], entityDataCurrentWaypointLocation[i]);
			}
			
			indexDataID++;
		}
	}
};
