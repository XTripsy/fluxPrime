#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "DrawDebugHelpers.h"
#include "FluxPrimeBaseSystems.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "NotifySystems/FluxPrimeNotifyAttackSystems.h"
#include "NotifySystems/FluxPrimeNotifyDeadSystems.h"
#include "NotifySystems/FluxPrimeNotifySpawnSFXSystems.h"
#include "NotifySystems/FluxPrimeNotifySpawnVFXSystems.h"
#include "FluxPrimeAnimationSystems.generated.h"

DECLARE_DELEGATE_OneParam(FOnCrowdsStateChange, const FInstancedStruct& payload);

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	bool IsDebug = false;
	
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CrowdsComponents;
	
	UPROPERTY()
	TArray<FFluxPrimeAnimationNotify> QueueAnimationNotifies;
	
	UPROPERTY()
	FFluxPrimeNotifyAttackSystems NotifyAttackSystems;
	
	UPROPERTY()
	FFluxPrimeNotifySpawnSFXSystems NotifySpawnSfxSystems;
	
	UPROPERTY()
	FFluxPrimeNotifySpawnVFXSystems NotifySpawnVfxSystems;
	
	UPROPERTY()
	FFluxPrimeNotifyDeadSystems NotifyDeadSystems;
	
public:
	FOnCrowdsStateChange OnCrowdsStateChange;
	
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
	
	bool IsNotifyTriggered(float PrevFrame, float CurrentFrame, float NotifyFrame)
	{
		if (PrevFrame < 0.0f) return false;
		
		if (CurrentFrame >= PrevFrame)
		{
			return NotifyFrame > PrevFrame &&
				   NotifyFrame <= CurrentFrame;
		}

		return NotifyFrame > PrevFrame ||
			   NotifyFrame <= CurrentFrame;
	}
	
	void PlayAnimation(TObjectPtr<UWorld> world, FFluxPrimeCrowds& members, int32 indexMembers)
	{
		uint8 indexAnimation = static_cast<uint8>(members.CrowdsAnimationState[indexMembers]);
		
		float startFrame = members.CrowdsAnimationMapping[indexMembers].AnimationData[indexAnimation].AnimationStart;
		float endFrame = members.CrowdsAnimationMapping[indexMembers].AnimationData[indexAnimation].AnimationEnd;
		
		float realTime = world->GetRealTimeSeconds();
		float localAnimTime = realTime - members.CrowdsStartTimeAnimation[indexMembers];
		float realTimeFrames = localAnimTime * 30.0f;
		float reminder = FMath::Fmod(realTimeFrames, (endFrame - startFrame) + 1.0f);
		float current = startFrame + reminder;
		
		if (IsDebug) ShowDebug(world, members, indexMembers, indexAnimation, current);
		
		PlayAnimationNotify(members.CrowdsAnimationMapping[indexMembers].AnimationData[indexAnimation].AnimationNotify, members.CrowdsID[indexMembers], current, members.CrowdsPreviousAnimationFrame[indexMembers]);
		
		members.CrowdsPreviousAnimationFrame[indexMembers] = current;
	}
	
	void PlayAnimationNotify(FFluxCrowdsAnimationNotify* notify, uint32 memberID, float currentFrame, float previousFrame)
	{
		for (int i = 0; i < FluxConfig::AnimationArrayCount; ++i)
		{
			UE_LOG(LogTemp, Log, TEXT("[NOTIFY] :: CEK :: Target[%d] Current[%f]"), notify[i].AnimationNotifyFrame, currentFrame);
			
			if (!IsNotifyTriggered(previousFrame, currentFrame, notify[i].AnimationNotifyFrame)) continue;

			QueueAnimationNotifies.Add(
				{
					memberID,
					notify[i].AnimationNotifyType
				});
		}
	}
	
	void ExecuteQueueAnimationNotify(FFluxPrimeCrowds& members, uint16 indexMember)
	{
		for (auto& pair : QueueAnimationNotifies)
		{
			NotifyAttackSystems.ExecuteNotify(members, indexMember, pair);
			NotifySpawnSfxSystems.ExecuteNotify(members, indexMember, pair);
			NotifySpawnVfxSystems.ExecuteNotify(members, indexMember, pair);
			NotifyDeadSystems.ExecuteNotify(members, indexMember, pair);
		}
		
		QueueAnimationNotifies.Reset();
	}
	
	void BroadcastChangeState(uint16 id, int8 type,	uint16 currentStartFrame, uint16 currentEndFrame, uint16 previousStartFrame, uint16 previousEndFrame,uint16 previousStartTime)
	{
		FFluxPrimeOnSwictAnimationPayload payload;
		payload.IdPayload = id;
		payload.TypePayload = type;
		payload.PreviousStartFrameAnimationPayload = previousStartFrame;
		payload.PreviousEndFrameAnimationPayload = previousEndFrame;
		payload.PreviousStartTimeAnimationPayload = previousStartTime;
		payload.CurrentStartFrameAnimationPayload = currentStartFrame;
		payload.CurrentEndFrameAnimationPayload = currentEndFrame;
		
		FInstancedStruct instancedStruct = FInstancedStruct::Make(payload);
		OnCrowdsStateChange.ExecuteIfBound(instancedStruct);
	}
	
	void UpdateAnimation(TObjectPtr<UWorld> world, FFluxPrimeCrowds& members, uint16 i)
	{
		if (!world) return;
		
		if (members.CrowdsAnimationState[i] != members.CrowdsState[i])
		{
			// identity
			uint8 id = members.CrowdsID[i];
			uint8 type = members.CrowdsType[i];
			
			// previous
			uint8 indexAnimation = static_cast<uint8>(members.CrowdsAnimationState[i]);
			uint16 previousStartFrame = members.CrowdsAnimationMapping[i].AnimationData[indexAnimation].AnimationStart;
			uint16 previousEndFrame = members.CrowdsAnimationMapping[i].AnimationData[indexAnimation].AnimationEnd;
			uint16 previousTime = members.CrowdsStartTimeAnimation[i];
			
			// change current state
			members.CrowdsAnimationState[i] = members.CrowdsState[i];
			
			// current
			indexAnimation = static_cast<uint8>(members.CrowdsAnimationState[i]);
			uint16 currentStartFrame = members.CrowdsAnimationMapping[i].AnimationData[indexAnimation].AnimationStart;
			uint16 currentEndFrame = members.CrowdsAnimationMapping[i].AnimationData[indexAnimation].AnimationEnd;
			
			BroadcastChangeState(id, type, currentStartFrame, currentEndFrame, previousStartFrame, previousEndFrame, previousTime);
			
			// update start time
			members.CrowdsStartTimeAnimation[i] = world->GetRealTimeSeconds();
			members.CrowdsPreviousAnimationFrame[i] = -1.0f;
		}
	}
	
public:
	void InitializedAnimationSystems(bool isDebug, TArray<TObjectPtr<UInstancedStaticMeshComponent>> crowdsComponents, uint32 totalMember)
	{
		IsDebug = isDebug;
		CrowdsComponents = crowdsComponents;
		QueueAnimationNotifies.Reserve(totalMember * FluxConfig::AnimationArrayCount);
	}
	
	void UpdateAnimationSystemsFrame(TObjectPtr<UWorld> world, FFluxPrimeCrowds& members, int32 activeMembers)
	{
		for (int i = 0; i < activeMembers; ++i)
		{
			UpdateAnimation(world, members, i);
			PlayAnimation(world, members, i);
			ExecuteQueueAnimationNotify(members, i);
		}
	}
	
	void EndPlayAnimationSystems()
	{
		OnCrowdsStateChange.Unbind();
	}
};