#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeMovementSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeMovementSystems
{
	GENERATED_BODY()

	void UpdateMovementSystems(double DeltaTime, FFluxPrimeCrowds& members, const int32 memberActive)
	{
		for (int i = 0; i < memberActive; ++i)
		{
			int8 indexNavigationPath = members.CrowdsIndexNavigationPath[i];
			FVector location = members.CrowdsLocation[i];
			location.Z = 0;
			FVector dir = members.CrowdsNavigationPath[i].LocationPaths[indexNavigationPath] - location;
			
			FRotator rot = dir.Rotation();
			members.CrowdsRotation[i] = FRotator::CompressAxisToByte(rot.Yaw);
			
			dir = dir.GetSafeNormal();
			FVector velocity = dir * members.CrowdsMaxSpeed[i];
			members.CrowdsVelocity[i] += velocity + members.CrowdsAcceleration[i];
			
			members.CrowdsVelocity[i] = members.CrowdsVelocity[i].GetClampedToMaxSize(members.CrowdsMaxSpeed[i]);
			
			members.CrowdsLocation[i] += members.CrowdsVelocity[i] * DeltaTime;
			members.CrowdsAcceleration[i] = FVector::ZeroVector;
			members.CrowdsVelocity[i] = FVector::ZeroVector;
		}
	}
};
