#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "DrawDebugHelpers.h"
#include "FluxPrimeBaseSystems.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "FluxPrimeAnimationSystems.generated.h"

DECLARE_DELEGATE_OneParam(FOnAttackNotify, int32 crowdID);
DECLARE_DELEGATE_OneParam(FOnSpawnVFXNotify, int32 crowdID);
DECLARE_DELEGATE_OneParam(FOnSpawnSFXNotify, int32 crowdID);

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	bool IsDebug = false;
	
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CrowdsComponents;
	
public:
	FOnAttackNotify OnAttackNotify;
	FOnSpawnSFXNotify OnSpawnSFXNotify;
	FOnSpawnVFXNotify OnSpawnVFXNotify;
	
private:
	void ShowDebug(TObjectPtr<UWorld> world, FFluxPrimeCrowds& members, int32 indexMembers, int32 animationIndex, float currentFrame)
	{
		FVector textLocation = members.CrowdsLocation[indexMembers] + (FVector::UpVector * FluxConfig::DebugLocationAnimation);
		FString debugData = FString::Printf(TEXT("Animation Index: %d \n Animation Frame: %f"), animationIndex, currentFrame);
		
		DrawDebugString(
			world,
			textLocation,
			debugData,
			nullptr,
			FColor::Yellow,
			0.0f,
			false,
			FluxConfig::DebugScaleFont
		);
	}
	
	void PlayAnimation(TObjectPtr<UWorld> world, FFluxPrimeCrowds& members, int32 indexMembers, int32 indexAnimation)
	{
		float startIndex = members.CrowdsAnimationData[indexMembers].AnimationOffset[FMath::Max(0, indexAnimation)];
		float endIndex = members.CrowdsAnimationData[indexMembers].AnimationOffset[FMath::Max(1, indexAnimation+1)];
			
		float realTime = world->GetRealTimeSeconds();
		float localAnimTime = realTime - members.CrowdsStartTimeAnimationFrame[indexMembers];
		float realTimeFrames = localAnimTime * 30.0f;
		float reminder = FMath::Fmod(realTimeFrames, (endIndex - startIndex) + 1.0f);
		float current = startIndex + reminder;
		
		if (IsDebug) ShowDebug(world, members, indexMembers, indexAnimation, current);
		
		if (!members.CrowdsAnimationData[indexMembers].AnimationLoop[FMath::Max(0, indexAnimation+1)] 
			&& FMath::IsNearlyEqual(endIndex, current, 1.0f))
		{
			members.CrowdsAnimationIndex[indexMembers] = -1;
			return;
		}
		
		PlayAnimationNotify(members, indexMembers, current);
		members.CrowdsCurrentAnimationFrame[indexMembers] = current;
	}
	
	void PlayAnimationNotify(FFluxPrimeCrowds& members, int32 indexMembers, float currentFrame)
	{
		FFluxCrowdsAnimationNotify& currentNotify = *members.CrowdsAnimationData[indexMembers].AnimationNotify;
		
		for (int i = 0; i < FluxConfig::AnimationArrayCount; ++i)
		{
			if (!FMath::IsNearlyEqual(currentNotify.AnimationNotifyFrame[i]+1, currentFrame, 0.5f)) continue;

			switch (currentNotify.AnimationNotifyType[i])
			{
			case EFluxCrowdAnimationNotify::NotifyNone:
				break;
			case EFluxCrowdAnimationNotify::NotifyAttack:
				UE_LOG(LogTemp, Error, TEXT("[NOTIFY] :: NOTIFY ATTACK : %f"), currentFrame);
				OnAttackNotify.ExecuteIfBound(members.CrowdsID[indexMembers]);
				break;
			case EFluxCrowdAnimationNotify::NotifySpawnSFX:
				UE_LOG(LogTemp, Error, TEXT("[NOTIFY] :: NOTIFY SPAWN SFX : %f"), currentFrame);
				OnSpawnSFXNotify.ExecuteIfBound(members.CrowdsID[indexMembers]);
				break;
			case EFluxCrowdAnimationNotify::NotifySpawnVFX:
				UE_LOG(LogTemp, Error, TEXT("[NOTIFY] :: NOTIFY SPAWN VFX : %f"), currentFrame);
				OnSpawnVFXNotify.ExecuteIfBound(members.CrowdsID[indexMembers]);
				break;
			}
		}
	}
	
public:
	void InitializedAnimationSystems(bool isDebug, TArray<TObjectPtr<UInstancedStaticMeshComponent>> crowdsComponents)
	{
		IsDebug = isDebug;
		CrowdsComponents = crowdsComponents;
	}
	
	void UpdateAnimationSystemsFrame(TObjectPtr<UWorld> world, FFluxPrimeCrowds& members, int32 activeMembers)
	{
		for (int i = 0; i < activeMembers; ++i)
		{
			int32 indexAnimation = members.CrowdsAnimationIndex[i];
			if (indexAnimation < 0) continue;
			
			PlayAnimation(world, members, i, indexAnimation);
		}
	}
	
	void SwitchAnimation(TObjectPtr<UWorld> world, FFluxPrimeCrowds& members, const int32 indexMembers)
	{
		int32 id = 0;
		int32 index = members.CrowdsID.IndexOfByKey(id);
		
		if (!members.CrowdsID.IsValidIndex(index)) return;

		members.CrowdsStartTimeAnimationFrame[index] = world->GetRealTimeSeconds();
		members.CrowdsAnimationIndex[index] += 2;
		uint32 indexAnimation = members.CrowdsAnimationIndex[index];
		
		int32 type = members.CrowdsType[index];
	
		CrowdsComponents[type]->SetCustomDataValue(
				id,
				0,
				world->GetRealTimeSeconds(),
				false
				);
	
		CrowdsComponents[type]->SetCustomDataValue(
				id,
				1,
				members.CrowdsAnimationData[index].AnimationOffset[indexAnimation],
				false
				);
	
		CrowdsComponents[type]->SetCustomDataValue(
				id,
				2,
				members.CrowdsAnimationData[index].AnimationOffset[indexAnimation+1],
				false
				);
	}
	
	void MontageAnimation(FFluxPrimeCrowds& members, const int32 indexMembers, const int32 indexAnimation)
	{
		int32 id = 0;
		int32 index = members.CrowdsID.IndexOfByKey(id);
		
		if (!members.CrowdsID.IsValidIndex(index) || indexAnimation > 8) return;

		members.CrowdsAnimationIndex[index] = indexAnimation;
	}
	
	void EndPlayAnimationSystems()
	{
		OnAttackNotify.Unbind();
		OnSpawnSFXNotify.Unbind();
		OnSpawnVFXNotify.Unbind();
	}
};