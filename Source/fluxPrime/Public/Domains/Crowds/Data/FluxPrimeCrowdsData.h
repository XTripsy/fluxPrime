// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Generics/Data/FluxPrimeNavigationData.h"
#include "FluxPrimeCrowdsData.generated.h"

class UFluxPrimeCrowdsIdentity;

USTRUCT(BlueprintType)
struct FFluxPrimeTargetCatalog
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TWeakObjectPtr<AActor> CrowdsTarget;
	
	UPROPERTY(EditAnywhere)
	uint16 CrowdsTargetPriority;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsLookup
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	int32 CrowdsID;
	
	UPROPERTY(EditAnywhere)
	int8 CrowdsType;
	
	bool operator==(const FFluxPrimeCrowdsLookup& Other) const
	{
		return CrowdsID == Other.CrowdsID && CrowdsType == Other.CrowdsType;
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowds
{
	GENERATED_BODY()
	
#pragma region IdentityData
	
	UPROPERTY(EditAnywhere)
	TArray<int16> CrowdsID;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsType;
	
	UPROPERTY(EditAnywhere)
	TArray<float> CrowdsMaxSpeed;
	
	UPROPERTY(EditAnywhere)
	TArray<float> CrowdsDamage;
	
	UPROPERTY(EditAnywhere)
	TArray<int16> CrowdsAbilityRange;
	
	UPROPERTY(EditAnywhere)
	TArray<int16> CrowdsHealth;
	
	UPROPERTY(EditAnywhere)
	TArray<int16> CrowdsDamageTaken;
	
	UPROPERTY(EditAnywhere)
	TArray<int16> CrowdsSize;
	
	UPROPERTY(EditAnywhere)
	TArray<int32> CrowdsCellID;

#pragma endregion
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsState;
	
	UPROPERTY(EditAnywhere)
	TArray<bool> CrowdsCondition;
	
	UPROPERTY(EditAnywhere)
	TArray<bool> CrowdsRequestBackToPool;
	
	UPROPERTY(EditAnywhere)
	TArray<bool> CrowdsRequestAbility;
	
#pragma region TransformData
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsLocation;
	
	UPROPERTY(EditAnywhere)
	TArray<float> CrowdsRotation;
	
#pragma endregion
	
#pragma region MovementData
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsVelocity;
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsAcceleration;
	
#pragma endregion
	
#pragma region AnimationData

	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsAnimationState;
	
	UPROPERTY(EditAnywhere)
	TArray<int32> CrowdsRequestAnimationNotify;
	
	UPROPERTY(EditAnywhere)
	TArray<float> CrowdsStartTimeAnimation;
	
	UPROPERTY(EditAnywhere)
	TArray<float> CrowdsPreviousAnimationFrame;
	
#pragma endregion 
	
#pragma region NavigationData
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsPreviousLocation;
	
	UPROPERTY(EditAnywhere)
	TArray<FFluxPrimeCrowdsCorridor> CrowdsCorridors;
	
	UPROPERTY(EditAnywhere)
	TArray<uint16> CrowdsTargetID;
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsTarget;
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsCurrentTarget;

	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsLastReplanTarget;
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsLastMoveTarget;

	UPROPERTY(EditAnywhere)
	TArray<float> CrowdsLastOptimizeTime;

	UPROPERTY(EditAnywhere)
	TArray<float> CrowdsLastMoveTargetTime;
	
	UPROPERTY(EditAnywhere)
	TArray<uint8> CrowdsCountCorridor;
	
	UPROPERTY(EditAnywhere)
	TArray<bool> CrowdsRequestNeedReplan;
	
	UPROPERTY(EditAnywhere)
	TArray<FFluxPrimeCrowdsWaypoint> CrowdsWaypoints;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsCountWaypoints;
	
#pragma endregion
	
	// perlu refactor
	void Init(int32 totalMember)
	{
		CrowdsLocation.Reserve(totalMember);
		CrowdsRotation.Reserve(totalMember);
		CrowdsVelocity.Reserve(totalMember);
		CrowdsAcceleration.Reserve(totalMember);
		CrowdsCellID.Reserve(totalMember);
		CrowdsCondition.Reserve(totalMember);
		CrowdsRequestBackToPool.Reserve(totalMember);
		CrowdsMaxSpeed.Reserve(totalMember);
		CrowdsType.Reserve(totalMember);
		CrowdsID.Reserve(totalMember);
		CrowdsHealth.Reserve(totalMember);
		CrowdsDamage.Reserve(totalMember);
		CrowdsDamageTaken.Reserve(totalMember);
		CrowdsAbilityRange.Reserve(totalMember);
		CrowdsRequestAbility.Reserve(totalMember);
		CrowdsSize.Reserve(totalMember);
		CrowdsState.Reserve(totalMember);
		CrowdsAnimationState.Reserve(totalMember);
		CrowdsRequestAnimationNotify.Reserve(totalMember);
		CrowdsStartTimeAnimation.Reserve(totalMember);
		CrowdsPreviousAnimationFrame.Reserve(totalMember);
		CrowdsPreviousLocation.Reserve(totalMember);
		CrowdsCorridors.Reserve(totalMember);
		CrowdsTargetID.Reserve(totalMember);
		CrowdsTarget.Reserve(totalMember);
		CrowdsCurrentTarget.Reserve(totalMember);
		CrowdsLastReplanTarget.Reserve(totalMember);
		CrowdsLastMoveTarget.Reserve(totalMember);
		CrowdsLastOptimizeTime.Reserve(totalMember);
		CrowdsLastMoveTargetTime.Reserve(totalMember);
		CrowdsCountCorridor.Reserve(totalMember);
		CrowdsRequestNeedReplan.Reserve(totalMember);
		CrowdsWaypoints.Reserve(totalMember);
		CrowdsCountWaypoints.Reserve(totalMember);
	}
};

FORCEINLINE uint32 GetTypeHash(const FFluxPrimeCrowdsLookup& Key)
{
	return HashCombine(GetTypeHash(Key.CrowdsID), GetTypeHash(Key.CrowdsType));
}
