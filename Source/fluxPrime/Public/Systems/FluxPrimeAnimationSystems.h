#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Cores/FluxPrimeStruct.h"
#include "DrawDebugHelpers.h"
#include "FluxPrimeAnimationSystems.generated.h"

DECLARE_DELEGATE_OneParam(FOnAttackNotify, int32 crowdID);
DECLARE_DELEGATE_OneParam(FOnSpawnVFXNotify, int32 crowdID);
DECLARE_DELEGATE_OneParam(FOnSpawnSFXNotify, int32 crowdID);

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationSystems
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	bool IsDebug = false;
	
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
	
	void PlayAnimation(TObjectPtr<UWorld> world, FFluxPrimeCrowds& members, int32 indexMembers, int32 indexAnimation, const TArray<TObjectPtr<UInstancedStaticMeshComponent>>& memberComponets)
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
		UpdateCustomDataValue(members, indexMembers, startIndex, current, memberComponets);
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
	
	void UpdateCustomDataValue(FFluxPrimeCrowds& members, int32 indexMembers, int32 startFrame, float currentFrame, const TArray<TObjectPtr<UInstancedStaticMeshComponent>>& memberComponets)
	{
		memberComponets[members.CrowdsType[indexMembers]]->SetCustomDataValue(
			members.CrowdsID[indexMembers],
			0,
			startFrame,
			false
			);
				
		memberComponets[members.CrowdsType[indexMembers]]->SetCustomDataValue(
			members.CrowdsID[indexMembers],
			1,
			currentFrame,
			false
			);
	}
	
public:
	void InitializedAnimationSystems(bool isDebug)
	{
		IsDebug = isDebug;
	}
	
	void UpdateAnimationSystemsFrame(TObjectPtr<UWorld> world, TStaticArray<FFluxPrimeCrowds, 2>& members, TArray<int32>& shortedIndex, int8& dataReadIndex, int32 activeMembers, const TArray<TObjectPtr<UInstancedStaticMeshComponent>>& memberComponets)
	{
		int8 writeIndex = (dataReadIndex + 1) % 2;
		FFluxPrimeCrowds& readBuffer = members[dataReadIndex];
		FFluxPrimeCrowds& writeBuffer = members[writeIndex];
		
		shortedIndex.SetNumUninitialized(activeMembers, EAllowShrinking::No);
		for (int i = 0; i < activeMembers; ++i)
		{
			shortedIndex[i] = i;
		}
        
		Algo::StableSort(shortedIndex, [&readBuffer](int32 a, int32 b)
			{
				return readBuffer.CrowdsType[a] < readBuffer.CrowdsType[b];
			}
		);
		
		for (int i = 0; i < activeMembers; ++i)
		{
			int32 tempShortedIndex = shortedIndex[i];
            
			writeBuffer.CrowdsLocation[i] = readBuffer.CrowdsLocation[tempShortedIndex];
			writeBuffer.CrowdsRotation[i] = readBuffer.CrowdsRotation[tempShortedIndex];
			writeBuffer.CrowdsAcceleration[i] = readBuffer.CrowdsAcceleration[tempShortedIndex];
			writeBuffer.CrowdsVelocity[i] = readBuffer.CrowdsVelocity[tempShortedIndex];
			writeBuffer.CrowdsID[i] = readBuffer.CrowdsID[tempShortedIndex];
			writeBuffer.CrowdsCellID[i] = readBuffer.CrowdsCellID[tempShortedIndex];
			writeBuffer.CrowdsMaxSpeed[i] = readBuffer.CrowdsMaxSpeed[tempShortedIndex];
			writeBuffer.CrowdsType[i] = readBuffer.CrowdsType[tempShortedIndex];
			writeBuffer.CrowdsHealth[i] = readBuffer.CrowdsHealth[tempShortedIndex];
			writeBuffer.CrowdsSize[i] = readBuffer.CrowdsSize[tempShortedIndex];
			writeBuffer.CrowdsDamage[i] = readBuffer.CrowdsDamage[tempShortedIndex];
			writeBuffer.CrowdsTargetLocation[i] = readBuffer.CrowdsTargetLocation[tempShortedIndex];
			writeBuffer.CrowdsIndexNavigationPath[i] = readBuffer.CrowdsIndexNavigationPath[tempShortedIndex];
			writeBuffer.CrowdsTotalNavigationPath[i] = readBuffer.CrowdsTotalNavigationPath[tempShortedIndex];
			writeBuffer.CrowdsNavigationPath[i] = readBuffer.CrowdsNavigationPath[tempShortedIndex];
			writeBuffer.CrowdsAnimationData[i] = readBuffer.CrowdsAnimationData[tempShortedIndex];
			writeBuffer.CrowdsAnimationIndex[i] = readBuffer.CrowdsAnimationIndex[tempShortedIndex];
			writeBuffer.CrowdsStartTimeAnimationFrame[i] = readBuffer.CrowdsStartTimeAnimationFrame[tempShortedIndex];
		}
		
		for (int i = 0; i < activeMembers; ++i)
		{
			int32 indexAnimation = writeBuffer.CrowdsAnimationIndex[i];
			if (indexAnimation < 0) continue;
			
			PlayAnimation(world, writeBuffer, i, indexAnimation, memberComponets);
		}
		
		dataReadIndex = writeIndex;
	}
	
	void SwitchAnimation(TObjectPtr<UWorld> world, FFluxPrimeCrowds& members, const int32 indexMembers)
	{
		int32 id = 0;
		int32 index = members.CrowdsID.IndexOfByKey(id);
		
		if (!members.CrowdsID.IsValidIndex(index)) return;

		members.CrowdsStartTimeAnimationFrame[index] = world->GetRealTimeSeconds();
		members.CrowdsAnimationIndex[index] += 2;
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