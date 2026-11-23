#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Framework/Systems/StateMachine/FluxPrimeBaseState.h"
#include "Generics/Data/FluxPrimeMeshOrientationConfig.h"
#include "UObject/ReflectedTypeAccessors.h"
#include "FluxPrimeCrowdsAbilityState.generated.h"

USTRUCT(BlueprintType)
struct FAbilityStateDataTagConfig
{
	GENERATED_BODY()

	FAbilityStateDataTagConfig()
	{
		EntityDataTargetLocationConfig.ExpectedStructType = TBaseStructure<FVector>::Get();
		EntityDataRotationConfig.ExpectedStructType = TBaseStructure<FRotator>::Get();
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataCurrentWaypointLocationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLocationConfig;

	/** Optional. Without it and the rotation tag, attacking entities keep their facing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataTargetLocationConfig;

	/** Optional. See EntityDataTargetLocationConfig. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRotationConfig;

	bool IsValid() const
	{
		// Aim is opt-in: only the hold inputs are required for the state to run.
		return EntityDataCurrentWaypointLocationConfig.EntityDataTag.IsValid() 
		   && EntityDataLocationConfig.EntityDataTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsAbilityState : public FFluxPrimeBaseState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FAbilityStateDataTagConfig DataTagConfig;

	/** Degrees per second the entity turns toward its target while attacking. 0 disables. */
	UPROPERTY(EditAnywhere, Category = "Ability", meta = (ClampMin = "0", Units = "DegreesPerSecond"))
	float AimTurnRateDegreesPerSecond = 180.0f;

	const FFluxPrimeMeshOrientationConfig* MeshOrientation = &FFluxPrimeMeshOrientationConfig::Default();

	TArray<int16> EntityDataCurrentWaypointLocationID, EntityDataLocationID, EntityDataRotationID, EntityDataTargetLocationID;
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores, const FFluxPrimeMeshOrientationConfig* meshOrientation)
	{
		if (meshOrientation) MeshOrientation = meshOrientation;

		int32 countData = dataStores.Num();
		EntityDataCurrentWaypointLocationID.Init(0, countData);
		EntityDataLocationID.Init(0, countData);
		EntityDataRotationID.Init(INDEX_NONE, countData);
		EntityDataTargetLocationID.Init(INDEX_NONE, countData);
		
		int32 indexDataId = 0;
		for (auto& dataStore : dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataCurrentWaypointLocationConfig, EntityDataCurrentWaypointLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataLocationConfig, EntityDataLocationID[indexDataId]);
			// Aim is opt-in, so a missing or mistyped aim input must not invalidate the hold write.
			TryResolvedOptionalData(dataStore, DataTagConfig.EntityDataRotationConfig, EntityDataRotationID[indexDataId]);
			TryResolvedOptionalData(dataStore, DataTagConfig.EntityDataTargetLocationConfig, EntityDataTargetLocationID[indexDataId]);

			indexDataId++;
		}
	}
	
	void ExecuteState(float deltaTime, int32 indexDataID, int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		if (!IsAllDataValid)
		{
			UE_LOG(LogFluxPrime, Verbose, TEXT("ABILITY STATE :: data invalid; system skipped"));
			return;
		}
		
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_State_Ability);
		
		FVector* entityDataCurrentWaypointLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointLocationID[indexDataID]));
		FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
		
		entityDataCurrentWaypointLocation[indexEntity] = entityDataLocation[indexEntity]; 

		if (EntityDataRotationID[indexDataID] == INDEX_NONE || EntityDataTargetLocationID[indexDataID] == INDEX_NONE) return;

		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_State_Ability_Aiming);
		FRotator* entityDataRotation = reinterpret_cast<FRotator*>(dataStore.GetRawBufferData(EntityDataRotationID[indexDataID]));
		FVector* entityDataTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]));

		FVector direction = entityDataTargetLocation[indexEntity] - entityDataLocation[indexEntity];
		direction.Z = 0.0f;
		if (direction.IsNearlyZero()) return;

		const float targetYaw = MeshOrientation->ToMeshYaw(direction.Rotation().Yaw);
		const float deltaYaw = FMath::FindDeltaAngleDegrees(entityDataRotation[indexEntity].Yaw, targetYaw);
		const float maxDelta = FMath::Max(AimTurnRateDegreesPerSecond, 0.0f) * deltaTime;
		entityDataRotation[indexEntity].Yaw += FMath::Clamp(deltaYaw, -maxDelta, maxDelta);
	}
};
