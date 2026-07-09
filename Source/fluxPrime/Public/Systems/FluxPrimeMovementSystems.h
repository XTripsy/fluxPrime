#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeBaseSystems.h"
#include "Cores/FluxPrimeEnum.h"
#include "FluxPrimeMovementSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeMovementSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool isDebug = false;
	
	UPROPERTY()
	TObjectPtr<UWorld> world; 
	TArray<EFluxPrimeCrowdState>* stateCrowds = nullptr;
	TArray<FVector>* locationCrowds = nullptr;
	TArray<float>* rotationCrowds = nullptr;
	TArray<FVector>* velocityCrowds = nullptr;
	TArray<FVector>* accelerationCrowds = nullptr;
	TArray<float>* maxSpeedCrowds = nullptr;
	TArray<FVector>* currentPathCrowds = nullptr;
	uint16* memberActive = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeMovementSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()

private:
	UPROPERTY()
	bool IsDebug = false;
	
	UPROPERTY()
	TObjectPtr<UWorld> World; 
	
	TArray<EFluxPrimeCrowdState>* StateCrowds = nullptr;
	TArray<FVector>* LocationCrowds = nullptr;
	TArray<float>* RotationCrowds = nullptr;
	TArray<FVector>* VelocityCrowds = nullptr;
	TArray<FVector>* AccelerationCrowds = nullptr;
	TArray<float>* MaxSpeedCrowds = nullptr;
	TArray<FVector>* CurrentPathCrowds = nullptr;
	uint16* MemberActive = nullptr;
	
private:
	void ShowDebug(const int32 indexMember, FVector direction)
	{
		auto& location = *LocationCrowds;
		auto& rotation = *RotationCrowds;
		
		float arrowLength = 50.0f;
		FVector startPos = location[indexMember] + (FVector::UpVector * FluxConfig::DebugLocationMovement);
		FVector endPos = startPos + (direction * arrowLength);

		DrawDebugDirectionalArrow(
			World,
			startPos,
			endPos,
			250.0f,
			FColor::Blue,
			false,
			-1.0f,
			0,
			5.0f
		);
		
		float currentYaw = rotation[indexMember] + 85;
		direction = FRotator(0, currentYaw, 0).Vector();
		arrowLength = 75.0f;
		startPos = location[indexMember] + (FVector::UpVector * (FluxConfig::DebugLocationMovement + 20.0f));
		endPos = startPos + (direction * arrowLength);
		
		DrawDebugDirectionalArrow(
			World,
			startPos,
			endPos,
			300.0f,
			FColor::Cyan,
			false,
			-1.0f,
			0,
			5.0f
		);
	}
	
	void ShowDebugNet(TObjectPtr<UWorld> world, TArray<FFluxPrimeCrowdsNet>& members, const int32 indexMember, FVector direction)
	{
		float arrowLength = 50.0f;
		FVector startPos = members[indexMember].NetLocation + (FVector::UpVector * FluxConfig::DebugLocationMovement);
		FVector endPos = startPos + (direction * arrowLength);

		DrawDebugDirectionalArrow(
			world,
			startPos,
			endPos,
			250.0f,
			FColor::Blue,
			false,
			-1.0f,
			0,
			5.0f
		);
	
		float currentYaw = FRotator::DecompressAxisFromByte(members[indexMember].NetRotation) + 85;
		direction = FRotator(0, currentYaw, 0).Vector();
		arrowLength = 75.0f;
		startPos = members[indexMember].NetLocation + (FVector::UpVector * (FluxConfig::DebugLocationMovement + 20.0f));
		endPos = startPos + (direction * arrowLength);
		
		DrawDebugDirectionalArrow(
			world,
			startPos,
			endPos,
			300.0f,
			FColor::Cyan,
			false,
			-1.0f,
			0,
			5.0f
		);
	}
	
public:
	void InitializedMovementSystems(FFluxPrimeMovementSystemsContext context)
	{
		check(context.world);
		check(context.locationCrowds);
		check(context.rotationCrowds);
		check(context.stateCrowds);
		check(context.accelerationCrowds);
		check(context.currentPathCrowds);
		check(context.velocityCrowds);
		check(context.maxSpeedCrowds);
		check(context.memberActive);
		
		IsDebug = context.isDebug;
		World = context.world;
		LocationCrowds = context.locationCrowds;
		RotationCrowds = context.rotationCrowds;
		StateCrowds = context.stateCrowds;
		AccelerationCrowds = context.accelerationCrowds;
		CurrentPathCrowds = context.currentPathCrowds;
		VelocityCrowds = context.velocityCrowds;
		MaxSpeedCrowds = context.maxSpeedCrowds;
		MemberActive = context.memberActive;
	}
	
	void UpdateMovementSystems(double DeltaTime)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Movement_Systems);
		
		auto& location = *LocationCrowds;
		auto& rotation = *RotationCrowds;
		auto& maxSpeed = *MaxSpeedCrowds;
		auto& velocity = *VelocityCrowds;
		auto& acceleration = *AccelerationCrowds;
		
		for (int i = 0; i < *MemberActive; ++i)
		{
			if ((*StateCrowds)[i] != EFluxPrimeCrowdState::StateWalk) continue;
			
			FVector currentLocation = location[i];
			currentLocation.Z = 0.0f;

			FVector dir = (*CurrentPathCrowds)[i] - currentLocation;
			dir.Normalize();
			if (IsDebug) ShowDebug(i, dir);

			if (!dir.IsNearlyZero())
			{	
				// perlu di ubah
				float targetYaw = dir.Rotation().Yaw - 85.0f;
				float deltaYaw = FMath::FindDeltaAngleDegrees(rotation[i], targetYaw);
				rotation[i] += FMath::Clamp(deltaYaw, -360 * DeltaTime, 360 * DeltaTime);
				float angleError = FMath::Abs(FMath::FindDeltaAngleDegrees(rotation[i], targetYaw));
				
				float moveFactor = FMath::GetMappedRangeValueClamped(FVector2D(120.f, 0.f), FVector2D(0.25f, 1.0f), angleError);
				FVector desiredVelocity = dir * maxSpeed[i] * moveFactor;
				FVector avoidanceVelocity = acceleration[i];
				FVector finalDesiredVelocity = desiredVelocity + avoidanceVelocity;
				
				const float alpha = FMath::Clamp(8.0f * DeltaTime, 0.f, 1.f);
				velocity[i] += (finalDesiredVelocity - velocity[i]) * alpha;
				velocity[i] = velocity[i].GetClampedToMaxSize(maxSpeed[i] * 1.2f);
				
				location[i] += velocity[i] * DeltaTime;
			}

			acceleration[i] = FVector::ZeroVector;
		}
	}
	
	void UpdateNetMovementSystems(TObjectPtr<UWorld> world, double DeltaTime, TArray<FFluxPrimeCrowdsNet>& members, TArray<FFluxPrimeCrowdsTargetNet>& targets, TArray<FFluxPrimeCrowdsAccelerationNet>& accelerations, const int32 memberActive)
	{
		for (int i = 0; i < memberActive; ++i)
		{
			FVector location = members[i].NetLocation;
			location.Z = 0;
			FVector dir = targets[i].NetTargetLocation - location;
			
			/*float distToTargetSq = dir.SizeSquared();
			if (distToTargetSq <= StoppingDistanceSq)
			{
				members[i].NetVelocity = FVector::ZeroVector;
				UE_LOG(LogTemp, Error, TEXT("-----------STOP"));
				continue;
			}*/
			
			FRotator rot = dir.Rotation();
			
			// forwad static mesh perlu di rubah agar tidak perlu manipulasi forwardnya
			float targetYaw = rot.Yaw - 85;
			float currentYaw = FRotator::DecompressAxisFromByte(members[i].NetRotation);
			FRotator currentRot(0.f, currentYaw, 0.f);
			FRotator targetRot(0.f, targetYaw, 0.f);
			float yaw = FMath::RInterpConstantTo(currentRot, targetRot, DeltaTime, 45.0f).Yaw;
			members[i].NetRotation = FRotator::CompressAxisToByte(yaw);
			
			dir = dir.GetSafeNormal();
			FVector velocity = dir * members[i].NetMaxSpeed;
			members[i].NetVelocity += velocity + accelerations[i].NetAcceleration;
			
			if (IsDebug) ShowDebugNet(world, members, i, dir);
			
			members[i].NetVelocity = members[i].NetVelocity.GetClampedToMaxSize(members[i].NetMaxSpeed);
			
			members[i].NetLocation += members[i].NetVelocity * DeltaTime;
			members[i].NetVelocity = FVector::ZeroVector;
		}
	}
};
