#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "GameplayTagContainer.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Generics/Data/FluxPrimeMeshOrientationConfig.h"
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

/**
* Limits crowd avoidance to a fraction of the steering velocity.
*
* Separation is authored per crowd and can exceed an entity's max speed (the shipped
* configuration uses 500 while the slower archetype moves at 75-225 cm/s). Applying that
* force unclamped lets avoidance reverse path following, which pushes agents off the
* navmesh and through walls. Keeping avoidance proportional to the desired velocity means
* the agent always keeps a steering component toward its waypoint.
*/
inline FVector ClampAvoidanceVelocity(const FVector& desiredVelocity, const FVector& avoidanceVelocity, const float avoidanceWeight)
{
	const float MaxAvoidance = desiredVelocity.Size() * FMath::Max(avoidanceWeight, 0.0f);
	return avoidanceVelocity.GetClampedToMaxSize(MaxAvoidance);
}

USTRUCT(BlueprintType)
struct FFluxPrimeMovementSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FMovementSystemDataTagConfig DataTagConfig;

	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = "0"))
	float SteeringResponsiveness = 8.0f;
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = "0.001", Units = "Degrees"))
	float SlowdownAngleDegrees = 120.0f;
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = "0", ClampMax = "1"))
	float MinMoveFactor = 0.25f;
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = "0"))
	float VelocityOverspeedFactor = 1.2f;
	/**
	 * Upper bound on avoidance relative to the desired steering velocity. Separation can be
	 * authored higher than max speed, so this keeps the waypoint direction authoritative.
	 */
	UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = "0"))
	float AvoidanceWeight = 0.5f;

	const FFluxPrimeMeshOrientationConfig* MeshOrientation = &FFluxPrimeMeshOrientationConfig::Default();

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
					float targetYaw = MeshOrientation->ToMeshYaw(dir.Rotation().Yaw);
					float deltaYaw = FMath::FindDeltaAngleDegrees(entityDataRotation[i].Yaw, targetYaw);
					entityDataRotation[i].Yaw += FMath::Clamp(deltaYaw, -360 * deltaTime, 360 * deltaTime);
					float angleError = FMath::Abs(FMath::FindDeltaAngleDegrees(entityDataRotation[i].Yaw, targetYaw));
					
					float moveFactor = FMath::GetMappedRangeValueClamped(FVector2D(SlowdownAngleDegrees, 0.f), FVector2D(MinMoveFactor, 1.0f), angleError);
					FVector desiredVelocity = dir * entityDataMaxSpeed[i].ValueFloat * moveFactor;
					FVector avoidanceVelocity = ClampAvoidanceVelocity(desiredVelocity, entityDataAcceleration[i], AvoidanceWeight);
					FVector finalDesiredVelocity = desiredVelocity + avoidanceVelocity;
					
					const float alpha = FMath::Clamp(SteeringResponsiveness * deltaTime, 0.f, 1.f);
					entityDataVelocity[i] += (finalDesiredVelocity - entityDataVelocity[i]) * alpha;
					entityDataVelocity[i] = entityDataVelocity[i].GetClampedToMaxSize(entityDataMaxSpeed[i].ValueFloat * VelocityOverspeedFactor);
					
					entityDataLocation[i] += entityDataVelocity[i] * deltaTime;
				}

				entityDataAcceleration[i] = FVector::ZeroVector;
			}
			
			indexDataID++;
		}
	}
};