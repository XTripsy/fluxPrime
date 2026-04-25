#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeMovementSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeMovementSystems
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
		
		
		float currentYaw = FRotator::DecompressAxisFromByte(members.CrowdsRotation[indexMember]) + 85;
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
	
public:
	void InitializedMovementSystems(bool isDebug)
	{
		IsDebug = isDebug;
	}
	
	void UpdateMovementSystems(TObjectPtr<UWorld> world, double DeltaTime, FFluxPrimeCrowds& members, const int32 memberActive)
	{
		for (int i = 0; i < memberActive; ++i)
		{
			int8 indexNavigationPath = members.CrowdsIndexNavigationPath[i];
			FVector location = members.CrowdsLocation[i];
			location.Z = 0;
			FVector dir = members.CrowdsNavigationPath[i].LocationPaths[indexNavigationPath] - location;
			
			FRotator rot = dir.Rotation();
			
			// forwad static mesh perlu di rubah agar tidak perlu manipulasi forwardnya
			float targetYaw = rot.Yaw - 85;
			float currentYaw = FRotator::DecompressAxisFromByte(members.CrowdsRotation[i]);
			FRotator currentRot(0.f, currentYaw, 0.f);
			FRotator targetRot(0.f, targetYaw, 0.f);
			float yaw = FMath::RInterpConstantTo(currentRot, targetRot, DeltaTime, 45.0f).Yaw;
			members.CrowdsRotation[i] = FRotator::CompressAxisToByte(yaw);
			
			dir = dir.GetSafeNormal();
			FVector velocity = dir * members.CrowdsMaxSpeed[i];
			members.CrowdsVelocity[i] += velocity + members.CrowdsAcceleration[i];
			
			if (IsDebug) ShowDebug(world, members, i, dir);
			
			members.CrowdsVelocity[i] = members.CrowdsVelocity[i].GetClampedToMaxSize(members.CrowdsMaxSpeed[i]);
			
			members.CrowdsLocation[i] += members.CrowdsVelocity[i] * DeltaTime;
			members.CrowdsAcceleration[i] = FVector::ZeroVector;
			members.CrowdsVelocity[i] = FVector::ZeroVector;
		}
	}
};
