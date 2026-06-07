#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeBaseSystems.h"
#include "Cores/FluxPrimeEnum.h"
#include "StructUtils/InstancedStruct.h"
#include "FluxPrimeMovementSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeMovementSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()

private:
	UPROPERTY()
	bool IsDebug = false;
	
private:
	void ShowDebug(TObjectPtr<UWorld> world, FFluxPrimeCrowds& members, const int32 indexMember, FVector direction)
	{
		float arrowLength = 50.0f;
		FVector startPos = members.CrowdsLocation[indexMember] + (FVector::UpVector * FluxConfig::DebugLocationMovement);
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
		
		float currentYaw = members.CrowdsRotation[indexMember] + 85;
		direction = FRotator(0, currentYaw, 0).Vector();
		arrowLength = 75.0f;
		startPos = members.CrowdsLocation[indexMember] + (FVector::UpVector * (FluxConfig::DebugLocationMovement + 20.0f));
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
	void InitializedMovementSystems(bool isDebug)
	{
		IsDebug = isDebug;
	}
	
	void UpdateMovementSystems(TObjectPtr<UWorld> world, double DeltaTime, FFluxPrimeCrowds& members, const int32 memberActive)
	{
		TArray<int32> queueCrowdsAnimation;
		queueCrowdsAnimation.Reserve(memberActive);
		
		for (int i = 0; i < memberActive; ++i)
		{
			if (members.CrowdsState[i] != EFluxPrimeCrowdState::StateWalk) continue;
			
			int8 indexNavigationPath = members.CrowdsIndexNavigationPath[i];
			FVector location = members.CrowdsLocation[i];
			location.Z = 0;
			FVector dir = members.CrowdsNavigationPath[i].LocationPaths[indexNavigationPath] - location;
			members.CrowdsCurrentTargetLocationPath[i] = members.CrowdsNavigationPath[i].LocationPaths[indexNavigationPath];
			
			dir = dir.GetSafeNormal();
			FVector desiredVelocity = dir * members.CrowdsMaxSpeed[i];
			FVector targetForce = (desiredVelocity - members.CrowdsVelocity[i]) * .4f;
			FVector totalForce = targetForce + members.CrowdsAcceleration[i];
			totalForce = totalForce.GetClampedToMaxSize(1500);
			members.CrowdsVelocity[i] += totalForce * DeltaTime;
			
			if (IsDebug) ShowDebug(world, members, i, dir);
			
			members.CrowdsVelocity[i] = members.CrowdsVelocity[i].GetClampedToMaxSize(members.CrowdsMaxSpeed[i] * 1.2f);
			members.CrowdsLocation[i] += members.CrowdsVelocity[i] * DeltaTime;
			
			UE_LOG(LogTemp, Log, TEXT("MOVEMENT SYSTEM:: ID %d, INDEX %d"), members.CrowdsID[i], i);
			
			// cek velocity is fast enough
			if (members.CrowdsVelocity[i].SizeSquared() > 10.0f) 
			{
				// rotation handle
				float targetYaw = members.CrowdsVelocity[i].Rotation().Yaw - 85;
				float currentYaw = members.CrowdsRotation[i];
				float deltaYaw = targetYaw - currentYaw;
				deltaYaw = FMath::UnwindDegrees(deltaYaw);
				float interpDelta = FMath::FInterpTo(0.0f, deltaYaw, DeltaTime, 55.0f);
				members.CrowdsRotation[i] = FMath::UnwindDegrees(currentYaw + interpDelta); 
			}
			members.CrowdsAcceleration[i] = FVector::ZeroVector;
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
