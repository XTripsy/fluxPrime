#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#if IF_WITH_EDITOR
#include "ProfilingDebugging/CpuProfilerTrace.h"
#endif
#include "FluxPrimeMovementSystem.generated.h"

USTRUCT(BlueprintType)
struct FMovementSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRotationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataAccelerationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataVelocityTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataCurrentWaypointTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataMaxSpeedTag;

	bool IsValid() const
	{
		return EntityDataLocationTag.IsValid() 
		   && EntityDataRotationTag.IsValid() 
		   && EntityDataAccelerationTag.IsValid() 
		   && EntityDataVelocityTag.IsValid() 
		   && EntityDataCurrentWaypointTag.IsValid() 
		   && EntityDataMaxSpeedTag.IsValid();
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
		for (auto& dataStore : dataStores)
		{
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataRotationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRotationTag));
			EntityDataMaxSpeedID.Add(dataStore.GetDataID(DataTagConfig.EntityDataMaxSpeedTag));
			EntityDataAccelerationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataAccelerationTag));
			EntityDataVelocityID.Add(dataStore.GetDataID(DataTagConfig.EntityDataVelocityTag));
			EntityDataCurrentWaypointID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCurrentWaypointTag));
		}
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
#if IF_WITH_EDITOR
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Movement_Systems);
#endif

		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{		
			FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
			FRotator* entityDataRotation = reinterpret_cast<FRotator*>(dataStore.GetRawBufferData(EntityDataRotationID[indexDataID]));
			FVector* entityDataAcceleration = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataAccelerationID[indexDataID]));
			FVector* entityDataVelocity = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataVelocityID[indexDataID]));
			FVector* entityDataCurrentWaypoint = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointID[indexDataID]));
			FFluxPrimeWrapperFloat* entityDataMaxSpeed = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataMaxSpeedID[indexDataID]));
			
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