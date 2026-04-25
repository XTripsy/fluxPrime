// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeEnum.h"
#include "FluxPrimeStruct.generated.h"

class UCrowdsIdentity;

namespace FluxConfig
{
	constexpr int8 AnimationArrayCount = 8;
	constexpr int8 NavigationArrayCount = 8;
	constexpr float DebugLocationIdentity = 500.0f;
	constexpr float DebugLocationAnimation = 420.0f;
	constexpr float DebugLocationNavigation = 340.0f;
	constexpr float DebugLocationMovement = 520.0f;
	constexpr float DebugScaleFont = .8f;
}

USTRUCT(BlueprintType)
struct FFluxCrowdsPath
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector LocationPaths[FluxConfig::NavigationArrayCount];
};

USTRUCT(BlueprintType)
struct FFluxCrowdsAnimationNotify
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	EFluxCrowdAnimationNotify AnimationNotifyType[FluxConfig::AnimationArrayCount] = {EFluxCrowdAnimationNotify::NotifyNone};
	
	UPROPERTY(EditAnywhere)
	int32 AnimationNotifyFrame[FluxConfig::AnimationArrayCount] = {-1};
};

USTRUCT(BlueprintType)
struct FFluxCrowdsAnimation
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	int32 AnimationOffset[FluxConfig::AnimationArrayCount] = {-1};
	
	UPROPERTY(EditAnywhere)
	FFluxCrowdsAnimationNotify AnimationNotify[FluxConfig::AnimationArrayCount];
	
	bool AnimationLoop[FluxConfig::AnimationArrayCount] = {true};
};

USTRUCT(BlueprintType)
struct FFluxPrimeMemberNet
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector_NetQuantize100 NetLocation;
	
	UPROPERTY(EditAnywhere)
	int8 NetRotation;
	
	UPROPERTY(EditAnywhere)
	int8 NetType;
	
	UPROPERTY(EditAnywhere)
	bool isActive;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowds
{
	GENERATED_BODY()
	
#pragma region IdentityData
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsID;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsType;
	
	UPROPERTY(EditAnywhere)
	TArray<float> CrowdsMaxSpeed;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsDamage;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsHealth;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsSize;
	
	UPROPERTY(EditAnywhere)
	TArray<int32> CrowdsCellID;
	
#pragma endregion
	
#pragma region TransformData
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsLocation;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsRotation;
	
#pragma endregion
	
#pragma region MovementData
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsVelocity;
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsAcceleration;
	
#pragma endregion
	
#pragma region AnimationData
	
	UPROPERTY(EditAnywhere)
	TArray<FFluxCrowdsAnimation> CrowdsAnimationData;
	
	UPROPERTY(EditAnywhere)
	TArray<int32> CrowdsAnimationIndex;
	
	UPROPERTY(EditAnywhere)
	TArray<float> CrowdsStartTimeAnimationFrame;
	
#pragma endregion 
	
#pragma region NavigationData
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsTargetLocation;
	
	UPROPERTY(EditAnywhere)
	TArray<FFluxCrowdsPath> CrowdsNavigationPath;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsIndexNavigationPath;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsTotalNavigationPath;
	
#pragma endregion
	
	void Init(int32 totalMember)
	{
		CrowdsLocation.Reserve(totalMember);
		CrowdsRotation.Reserve(totalMember);
		CrowdsVelocity.Reserve(totalMember);
		CrowdsAcceleration.Reserve(totalMember);
		CrowdsCellID.Reserve(totalMember);
		CrowdsMaxSpeed.Reserve(totalMember);
		CrowdsType.Reserve(totalMember);
		CrowdsID.Reserve(totalMember);
		CrowdsHealth.Reserve(totalMember);
		CrowdsDamage.Reserve(totalMember);
		CrowdsSize.Reserve(totalMember);
		CrowdsNavigationPath.Reserve(totalMember);
		CrowdsIndexNavigationPath.Reserve(totalMember);
		CrowdsTotalNavigationPath.Reserve(totalMember);
		CrowdsTargetLocation.Reserve(totalMember);
		CrowdsAnimationData.Reserve(totalMember);
		CrowdsAnimationIndex.Reserve(totalMember);
		CrowdsStartTimeAnimationFrame.Reserve(totalMember);
	}
};

USTRUCT(BlueprintType)
struct FFluxCatalogCrowds
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	TObjectPtr<UCrowdsIdentity> CrowdsIdentity;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	int32 CrowdsTotal;
};
