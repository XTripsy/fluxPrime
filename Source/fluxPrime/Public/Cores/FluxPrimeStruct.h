// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeEnum.h"
#include "Detour/DetourNavMesh.h"
//#include "Crowds/Identity/FluxPrimeCrowdsIdentity.h"
#include "FluxPrimeStruct.generated.h"

class UFluxPrimeCrowdsIdentity;
enum class EFluxPrimeCrowdState : uint8;

namespace FluxConfig
{
	constexpr int8 AnimationArrayCount = 8;
	constexpr int8 NavigationArrayCount = 8;
	constexpr int8 WaypointsCount = 16;
	constexpr int8 CorridorsCount = 64;
	constexpr float DebugLocationMovement = 620.0f;
	constexpr float DebugLocationIdentity = 600.0f;
	constexpr float DebugLocationAnimation = 520.0f;
	constexpr float DebugLocationNavigation = 440.0f;
	constexpr float DebugLocationSpatialGrid = 360.0f;
	constexpr float DebugScaleFont = .8f;
}

USTRUCT()
struct FFluxPrimeSpawnActionPayload
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsIdentity> Identity;
	
	UPROPERTY()
	FVector Location;
	
	UPROPERTY()
	FRotator Rotation;
};

USTRUCT()
struct FFluxPrimeDamageActionPayload
{
	GENERATED_BODY()
	
	UPROPERTY()
	int16 CrowdID;
	
	UPROPERTY()
	int8 CrowdType;
};

USTRUCT()
struct FFluxPrimeChangeTargetActionPayload
{
	GENERATED_BODY()
	
	UPROPERTY()
	int16 CrowdID;
	
	UPROPERTY()
	int8 CrowdType;
	
	UPROPERTY()
	uint16 TargetID;
	
	UPROPERTY()
	FVector NewTargetLocation;
};

USTRUCT()
struct FFluxPrimeRegisterTargetActionPayload
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> NewCrowdsTarget;
};

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
struct FFluxPrimeCrowdsWaypoint
{
	GENERATED_BODY()
	
	TStaticArray<FVector, FluxConfig::WaypointsCount> Waypoints;
	TStaticArray<dtPolyRef, FluxConfig::WaypointsCount> PolyRefs;
	TStaticArray<uint8, FluxConfig::WaypointsCount> Flags;
};

USTRUCT()
struct FFluxPrimeCrowdsCorridor
{
	GENERATED_BODY()

	TStaticArray<NavNodeRef, FluxConfig::CorridorsCount> Corridors;
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
struct FFluxPrimeRuntimeAnimationNotifyData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FFluxCrowdsAnimationNotify AnimationNotify[FluxConfig::AnimationArrayCount];
	
	UPROPERTY()
	int8 AnimationNotifyCount;
};

USTRUCT(BlueprintType)
struct FFluxPrimeRuntimeAnimationNotify
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FFluxPrimeRuntimeAnimationNotifyData RuntimeAnimationNotifyData[static_cast<int32>(EFluxPrimeCrowdState::Count)];
};

USTRUCT(BlueprintType)
struct FFluxPrimeRuntimeAnimationMapping
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	uint32 AnimationStart[static_cast<int32>(EFluxPrimeCrowdState::Count)];
	
	UPROPERTY(EditAnywhere)
	uint32 AnimationEnd[static_cast<int32>(EFluxPrimeCrowdState::Count)];
	
	bool AnimationForceNotify[static_cast<int32>(EFluxPrimeCrowdState::Count)];
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
	TArray<int8> CrowdsHealth;
	
	UPROPERTY(EditAnywhere)
	TArray<int16> CrowdsSize;
	
	UPROPERTY(EditAnywhere)
	TArray<int32> CrowdsCellID;

#pragma endregion
	
	UPROPERTY(EditAnywhere)
	TArray<EFluxPrimeCrowdState> CrowdsState;
	
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
	TArray<EFluxPrimeCrowdState> CrowdsAnimationState;
	
	UPROPERTY(EditAnywhere)
	TArray<EFluxPrimeCrowdAnimationNotify> CrowdsRequestAnimationNotify;
	
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

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsCatalog
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	TObjectPtr<UFluxPrimeCrowdsIdentity> CrowdsIdentity;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	int32 CrowdsTotal;
};

FORCEINLINE uint32 GetTypeHash(const FFluxPrimeCrowdsLookup& Key)
{
	return HashCombine(GetTypeHash(Key.CrowdsID), GetTypeHash(Key.CrowdsType));
}
