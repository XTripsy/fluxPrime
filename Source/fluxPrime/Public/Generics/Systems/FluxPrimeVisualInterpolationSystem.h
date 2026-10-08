#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "FluxPrimeVisualInterpolationSystem.generated.h"

USTRUCT(BlueprintType)
struct FVisualInterpolationSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLocationTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRotationTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataCurrentWaypointLocationTag;

	bool IsValid() const
	{
		return EntityDataLocationTag.IsValid()
		   && EntityDataRotationTag.IsValid()
		   && EntityDataCurrentWaypointLocationTag.IsValid(); 
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
		for (auto& dataStore : dataStores)
		{
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataRotationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRotationTag));
			EntityDataCurrentWaypointLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCurrentWaypointLocationTag));
		}
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_VisualInterpolation_Systems);
		
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
