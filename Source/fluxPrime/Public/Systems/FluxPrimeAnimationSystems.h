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
	
	void UpdateAnimationSystemsFrame(TObjectPtr<UWorld> world, FFluxPrimeCrowds members[2], TArray<int32>& shortedIndex, int8& dataReadIndex, int32 activeMembers, const TArray<TObjectPtr<UInstancedStaticMeshComponent>>& memberComponets)
	{
		int8 WriteIndex = (dataReadIndex + 1) % 2;
		FFluxPrimeCrowds& ReadBuffer = members[dataReadIndex];
		FFluxPrimeCrowds& WriteBuffer = members[WriteIndex];
		
		shortedIndex.SetNumUninitialized(activeMembers, EAllowShrinking::No);
		for (int i = 0; i < activeMembers; ++i)
		{
			shortedIndex[i] = i;
		}
        
		Algo::StableSort(shortedIndex, [&ReadBuffer](int32 a, int32 b)
			{
				return ReadBuffer.CrowdsType[a] < ReadBuffer.CrowdsType[b];
			}
		);
		
		for (int i = 0; i < activeMembers; ++i)
		{
			int32 tempShortedIndex = shortedIndex[i];
            
			WriteBuffer.CrowdsLocation[i] = ReadBuffer.CrowdsLocation[tempShortedIndex];
			WriteBuffer.CrowdsRotation[i] = ReadBuffer.CrowdsRotation[tempShortedIndex];
			WriteBuffer.CrowdsAcceleration[i] = ReadBuffer.CrowdsAcceleration[tempShortedIndex];
			WriteBuffer.CrowdsVelocity[i] = ReadBuffer.CrowdsVelocity[tempShortedIndex];
			WriteBuffer.CrowdsID[i] = ReadBuffer.CrowdsID[tempShortedIndex];
			WriteBuffer.CrowdsCellID[i] = ReadBuffer.CrowdsCellID[tempShortedIndex];
			WriteBuffer.CrowdsMaxSpeed[i] = ReadBuffer.CrowdsMaxSpeed[tempShortedIndex];
			WriteBuffer.CrowdsType[i] = ReadBuffer.CrowdsType[tempShortedIndex];
			WriteBuffer.CrowdsHealth[i] = ReadBuffer.CrowdsHealth[tempShortedIndex];
			WriteBuffer.CrowdsSize[i] = ReadBuffer.CrowdsSize[tempShortedIndex];
			WriteBuffer.CrowdsDamage[i] = ReadBuffer.CrowdsDamage[tempShortedIndex];
			WriteBuffer.CrowdsTargetLocation[i] = ReadBuffer.CrowdsTargetLocation[tempShortedIndex];
			WriteBuffer.CrowdsIndexNavigationPath[i] = ReadBuffer.CrowdsIndexNavigationPath[tempShortedIndex];
			WriteBuffer.CrowdsTotalNavigationPath[i] = ReadBuffer.CrowdsTotalNavigationPath[tempShortedIndex];
			WriteBuffer.CrowdsNavigationPath[i] = ReadBuffer.CrowdsNavigationPath[tempShortedIndex];
			WriteBuffer.CrowdsAnimationData[i] = ReadBuffer.CrowdsAnimationData[tempShortedIndex];
			WriteBuffer.CrowdsAnimationIndex[i] = ReadBuffer.CrowdsAnimationIndex[tempShortedIndex];
			WriteBuffer.CrowdsStartTimeAnimationFrame[i] = ReadBuffer.CrowdsStartTimeAnimationFrame[tempShortedIndex];
		}
		
		for (int i = 0; i < activeMembers; ++i)
		{
			int32 indexAnimation = WriteBuffer.CrowdsAnimationIndex[i];
			if (indexAnimation < 0) continue;
			
			PlayAnimation(world, WriteBuffer, i, indexAnimation, memberComponets);
		}
		
		dataReadIndex = WriteIndex;
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