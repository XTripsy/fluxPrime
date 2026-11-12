#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "GameplayTagContainer.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeMovementSystem.generated.h"

USTRUCT(BlueprintType)
struct FMovementSystemDataTagConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLocationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRotationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataAccelerationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataVelocityConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataCurrentWaypointConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataMaxSpeedConfig;

	bool IsValid() const
	{
		return EntityDataLocationConfig.EntityDataTag.IsValid() 
		   && EntityDataRotationConfig.EntityDataTag.IsValid() 
		   && EntityDataAccelerationConfig.EntityDataTag.IsValid() 
		   && EntityDataVelocityConfig.EntityDataTag.IsValid() 
		   && EntityDataCurrentWaypointConfig.EntityDataTag.IsValid() 
		   && EntityDataMaxSpeedConfig.EntityDataTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeMovementSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FMovementSystemDataTagConfig DataTagConfig;

	TArray<int16> EntityDataLocationID, EntityDataRotationID, EntityDataAccelerationID, EntityDataVelocityID, EntityDataCurrentWaypointID, EntityDataMaxSpeedID;
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		int32 countData = dataStores.Num();
		EntityDataLocationID.Init(0, countData);
		EntityDataRotationID.Init(0, countData);
		EntityDataAccelerationID.Init(0, countData);
		EntityDataVelocityID.Init(0, countData);
		EntityDataCurrentWaypointID.Init(0, countData);
		EntityDataMaxSpeedID.Init(0, countData);
		
		int32 indexDataId = 0;
		for (auto& dataStore : dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataLocationConfig, EntityDataLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRotationConfig, EntityDataRotationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataAccelerationConfig, EntityDataAccelerationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataVelocityConfig, EntityDataVelocityID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataCurrentWaypointConfig, EntityDataCurrentWaypointID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataMaxSpeedConfig, EntityDataMaxSpeedID[indexDataId]);
			indexDataId++;
		}
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Movement_Systems);
		
		if (!IsAllDataValid)
		{
			UE_LOG(LogFluxPrime, Verbose, TEXT("MOVEMENT SYSTEM :: data invalid; system skipped"));
			return;
		}

		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
			FRotator* entityDataRotation = reinterpret_cast<FRotator*>(dataStore.GetRawBufferData(EntityDataRotationID[indexDataID]));
			FVector* entityDataAcceleration = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataAccelerationID[indexDataID]));
			FVector* entityDataVelocity = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataVelocityID[indexDataID]));
			FVector* entityDataCurrentWaypoint = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointID[indexDataID]));
			FFluxPrimeWrapperFloat* entityDataMaxSpeed = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataMaxSpeedID[indexDataID]));
			
			check(entityDataLocation);
			check(entityDataRotation);
			check(entityDataAcceleration);
			check(entityDataVelocity);
			check(entityDataCurrentWaypoint);
			check(entityDataMaxSpeed);
			
			for (int i = 0; i < dataStore.GetActiveEntityCount(); ++i)
			{
				FVector currentLocation = entityDataLocation[i];
				currentLocation.Z = 0.0f;
				FVector currentTarget = entityDataCurrentWaypoint[i];
				currentLocation.Z = 0.0f;

				FVector dir = currentTarget - currentLocation;
				dir.Z = 0;
				dir.Normalize();

				if (!dir.IsNearlyZero())
				{	
					// perlu di ubah
					float targetYaw = dir.Rotation().Yaw - 85.0f;
					float deltaYaw = FMath::FindDeltaAngleDegrees(entityDataRotation[i].Yaw, targetYaw);
					entityDataRotation[i].Yaw += FMath::Clamp(deltaYaw, -360 * deltaTime, 360 * deltaTime);
					float angleError = FMath::Abs(FMath::FindDeltaAngleDegrees(entityDataRotation[i].Yaw, targetYaw));
					
					float moveFactor = FMath::GetMappedRangeValueClamped(FVector2D(120.f, 0.f), FVector2D(0.25f, 1.0f), angleError);
					FVector desiredVelocity = dir * entityDataMaxSpeed[i].ValueFloat * moveFactor;
					FVector avoidanceVelocity = entityDataAcceleration[i];
					FVector finalDesiredVelocity = desiredVelocity + avoidanceVelocity;
					
					const float alpha = FMath::Clamp(8.0f * deltaTime, 0.f, 1.f);
					entityDataVelocity[i] += (finalDesiredVelocity - entityDataVelocity[i]) * alpha;
					entityDataVelocity[i] = entityDataVelocity[i].GetClampedToMaxSize(entityDataMaxSpeed[i].ValueFloat * 1.2f);
					
					entityDataLocation[i] += entityDataVelocity[i] * deltaTime;
				}

				entityDataAcceleration[i] = FVector::ZeroVector;
			}
			
			indexDataID++;
		}
	}
};