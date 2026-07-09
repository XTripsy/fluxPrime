// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeEnum.h"
#include "GameplayTagContainer.h"
#include "FluxPrimeStruct.generated.h"

class UCrowdsIdentity;
enum class EFluxPrimeCrowdState : uint8;

namespace FluxConfig
{
	constexpr int8 AnimationArrayCount = 8;
	constexpr int8 NavigationArrayCount = 8;
	constexpr float DebugLocationMovement = 620.0f;
	constexpr float DebugLocationIdentity = 600.0f;
	constexpr float DebugLocationAnimation = 520.0f;
	constexpr float DebugLocationNavigation = 440.0f;
	constexpr float DebugLocationSpatialGrid = 360.0f;
	constexpr float DebugScaleFont = .8f;
}

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
struct FFluxPrimeCrowdsPath
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector LocationPaths[FluxConfig::NavigationArrayCount];
};

USTRUCT(BlueprintType)
struct FFluxPrimeOnSpawnCrowdsNetPayload
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector LocationPayload;
	
	UPROPERTY(EditAnywhere)
	FVector TargetLocationPayload;
	
	UPROPERTY(EditAnywhere)
	FVector AccelerationPayload;
	
	UPROPERTY(EditAnywhere)
	uint16 IdPayload;
	
	UPROPERTY(EditAnywhere)
	uint16 IndexSelectedPayload;
	
	UPROPERTY(EditAnywhere)
	int16 MaxSpeedPayload;
	
	UPROPERTY(EditAnywhere)
	uint16 StartFramePayload;
	
	UPROPERTY(EditAnywhere)
	uint16 EndFramePayload;
	
	UPROPERTY(EditAnywhere)
	int8 RotationPayload;
	
	UPROPERTY(EditAnywhere)
	int8 TypePayload;
};

USTRUCT(BlueprintType)
struct FFluxPrimeOnSwictAnimationPayload
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	uint16 IdPayload;
	
	UPROPERTY(EditAnywhere)
	uint16 PreviousStartFrameAnimationPayload;
	
	UPROPERTY(EditAnywhere)
	uint16 PreviousEndFrameAnimationPayload;
	
	UPROPERTY(EditAnywhere)
	uint16 PreviousStartTimeAnimationPayload;
	
	UPROPERTY(EditAnywhere)
	uint16 CurrentStartFrameAnimationPayload;
	
	UPROPERTY(EditAnywhere)
	uint16 CurrentEndFrameAnimationPayload;
	
	UPROPERTY(EditAnywhere)
	int8 TypePayload;
};

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationNotify
{
	GENERATED_BODY()
	
	UPROPERTY()
	uint32 CrowdIDNotify;
	
	UPROPERTY()
	EFluxPrimeCrowdAnimationNotify CrowdTypeNotify;
	
};

USTRUCT(BlueprintType)
struct FFluxCrowdsAnimationNotify
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	EFluxPrimeCrowdAnimationNotify AnimationNotifyType = EFluxPrimeCrowdAnimationNotify::NotifyNone;
	
	UPROPERTY(EditAnywhere)
	int32 AnimationNotifyFrame = 0;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsAnimationMapping
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FFluxCrowdsAnimationNotify AnimationNotify[FluxConfig::AnimationArrayCount];
	
	UPROPERTY(EditAnywhere)
	uint32 AnimationStart;
	
	UPROPERTY(EditAnywhere)
	uint32 AnimationEnd;
	
	UPROPERTY(EditAnywhere)
	bool AnimationLoop;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsAnimation
{
	GENERATED_BODY()
	
	TStaticArray<FFluxPrimeCrowdsAnimationMapping, static_cast<int32>(EFluxPrimeCrowdState::Count)> AnimationData;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsNet
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector_NetQuantize100 NetLocation;
	
	UPROPERTY(EditAnywhere)
	FVector_NetQuantize100 NetVelocity;
	
	UPROPERTY(EditAnywhere)
	uint16 NetMaxSpeed;
	
	UPROPERTY(EditAnywhere)
	uint16 NetID;
	
	UPROPERTY(EditAnywhere)
	int8 NetRotation;
	
	UPROPERTY(EditAnywhere)
	uint8 NetType;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsTargetNet
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector_NetQuantize100 NetTargetLocation;
	
	UPROPERTY(EditAnywhere)
	uint16 NetID;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsAccelerationNet
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector_NetQuantize100 NetAcceleration;
	
	UPROPERTY(EditAnywhere)
	uint16 NetID;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsAnimationNet
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	uint16 NetID;
	
	UPROPERTY(EditAnywhere)
	uint16 NetStart;
	
	UPROPERTY(EditAnywhere)
	uint16 NetEnd;
	
	UPROPERTY(EditAnywhere)
	uint8 NetType;
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
	TArray<int8> CrowdsDamage;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsHealth;
	
	UPROPERTY(EditAnywhere)
	TArray<int16> CrowdsSize;
	
	UPROPERTY(EditAnywhere)
	TArray<int32> CrowdsCellID;
	
	UPROPERTY(EditAnywhere)
	TArray<bool> CrowdsCondition;
	
	UPROPERTY(EditAnywhere)
	TArray<bool> CrowdsRequestBackToPool;
	
#pragma endregion
	
	UPROPERTY(EditAnywhere)
	TArray<EFluxPrimeCrowdState> CrowdsState;
	
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
	TArray<FFluxPrimeCrowdsAnimation> CrowdsAnimationMapping;

	UPROPERTY(EditAnywhere)
	TArray<EFluxPrimeCrowdState> CrowdsAnimationState;
	
	UPROPERTY(EditAnywhere)
	TArray<float> CrowdsStartTimeAnimation;
	
	UPROPERTY(EditAnywhere)
	TArray<float> CrowdsPreviousAnimationFrame;
	
#pragma endregion 
	
#pragma region NavigationData
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsCurrentTargetLocationPath;
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsTargetLocation;
	
	UPROPERTY(EditAnywhere)
	TArray<FFluxPrimeCrowdsPath> CrowdsNavigationPath;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsIndexNavigationPath;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsTotalNavigationPath;
	
	UPROPERTY(EditAnywhere)
	TArray<bool> CrowdsRequestNavigationPath;
	
#pragma endregion
	
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
		CrowdsSize.Reserve(totalMember);
		CrowdsState.Reserve(totalMember);
		CrowdsNavigationPath.Reserve(totalMember);
		CrowdsIndexNavigationPath.Reserve(totalMember);
		CrowdsTotalNavigationPath.Reserve(totalMember);
		CrowdsRequestNavigationPath.Reserve(totalMember);
		CrowdsTargetLocation.Reserve(totalMember);
		CrowdsCurrentTargetLocationPath.Reserve(totalMember);
		CrowdsAnimationMapping.Reserve(totalMember);
		CrowdsAnimationState.Reserve(totalMember);
		CrowdsStartTimeAnimation.Reserve(totalMember);
		CrowdsPreviousAnimationFrame.Reserve(totalMember);
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsCatalog
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	TObjectPtr<UCrowdsIdentity> CrowdsIdentity;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	int32 CrowdsTotal;
};

FORCEINLINE uint32 GetTypeHash(const FFluxPrimeCrowdsLookup& Key)
{
	return HashCombine(
		GetTypeHash(Key.CrowdsID),
		GetTypeHash(Key.CrowdsType));
}
