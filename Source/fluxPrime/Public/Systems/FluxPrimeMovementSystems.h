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
	//TStaticArray<FFluxPrimeCrowds, 2>* members = nullptr;
	FFluxPrimeCrowds* members = nullptr;
	//int8* dataReadIndex = nullptr;
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
	
	//TStaticArray<FFluxPrimeCrowds, 2>* Members = nullptr;
	FFluxPrimeCrowds* Members = nullptr;
	//int8* DataReadIndex = nullptr;
	uint16* MemberActive = nullptr;
	
private:
	void ShowDebug(const int32 indexMember, FVector direction)
	{
		//auto& members = (*Members)[*DataReadIndex];
		auto& members = *Members;
		
		float arrowLength = 50.0f;
		FVector startPos = members.CrowdsLocation[indexMember] + (FVector::UpVector * FluxConfig::DebugLocationMovement);
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
		
		float currentYaw = members.CrowdsRotation[indexMember] + 85;
		direction = FRotator(0, currentYaw, 0).Vector();
		arrowLength = 75.0f;
		startPos = members.CrowdsLocation[indexMember] + (FVector::UpVector * (FluxConfig::DebugLocationMovement + 20.0f));
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
		check(context.members);
		check(context.memberActive);
		//check(context.dataReadIndex);
		
		IsDebug = context.isDebug;
		World = context.world;
		Members = context.members;
		MemberActive = context.memberActive;
		//DataReadIndex = context.dataReadIndex;
	}
	
	void UpdateMovementSystems(double DeltaTime)
	{
		//auto& members = (*Members)[*DataReadIndex];
		auto& members = *Members;
		
		for (int i = 0; i < *MemberActive; ++i)
		{
			if (members.CrowdsState[i] != EFluxPrimeCrowdState::StateWalk) continue;
			
			int8 indexNavigationPath = members.CrowdsIndexNavigationPath[i];
			FVector location = members.CrowdsLocation[i];
			location.Z = 0.0f;

			FVector dir = members.CrowdsNavigationPath[i].LocationPaths[indexNavigationPath] - location;
			dir.Normalize();
			if (IsDebug) ShowDebug(i, dir);
			// current ini untuk net
			//members.CrowdsCurrentTargetLocationPath[i] = members.CrowdsNavigationPath[i].LocationPaths[indexNavigationPath];

			UE_LOG(LogTemp, Log, TEXT("MOVEMENT SYSTEMS :: CURRENT PATH %s"), *members.CrowdsNavigationPath[i].LocationPaths[indexNavigationPath].ToString());
			
			if (!dir.IsNearlyZero())
			{	
				float targetYaw = dir.Rotation().Yaw - 85.0f;
				members.CrowdsRotation[i] = FMath::RInterpTo(FRotator(0.f, members.CrowdsRotation[i], 0.f), FRotator(0.f, targetYaw, 0.f), DeltaTime, 10.0f).Yaw;
				float angleError = FMath::Abs(FMath::FindDeltaAngleDegrees(members.CrowdsRotation[i], targetYaw));
				
				float moveFactor = FMath::GetMappedRangeValueClamped(FVector2D(120.f, 0.f), FVector2D(0.25f, 1.0f), angleError);
				FVector desiredVelocity = dir * members.CrowdsMaxSpeed[i] * moveFactor;
				FVector avoidanceVelocity = members.CrowdsAcceleration[i];
				FVector finalDesiredVelocity = desiredVelocity + avoidanceVelocity;
				
				members.CrowdsVelocity[i] = FMath::VInterpTo(members.CrowdsVelocity[i], finalDesiredVelocity, DeltaTime, 8.0f);
				members.CrowdsVelocity[i] = members.CrowdsVelocity[i].GetClampedToMaxSize(members.CrowdsMaxSpeed[i] * 1.2f);
				
				members.CrowdsLocation[i] += members.CrowdsVelocity[i] * DeltaTime;
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
