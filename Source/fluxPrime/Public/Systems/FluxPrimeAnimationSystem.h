#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeSpatialGridSystem.h"
#include "DrawDebugHelpers.h"
#include "FluxPrimeBaseSystem.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeAnimationSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool isDebug = false;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* crowdsComponents = nullptr;
	TArray<FName>* crowdsCatalogName = nullptr;
	uint16 crowdsCountCatalog;
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>>* crowdsAnimationSoftRef = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	uint32 totalMember = 0;
	
	TArray<int16>* instanceIndexCrowds = nullptr;
	TArray<int8>* typeCrowds = nullptr;
	TArray<int32>* crowdsCellID = nullptr;
	TArray<EFluxPrimeCrowdState>* stateCrowds = nullptr;
	TArray<EFluxPrimeCrowdState>* animationStateCrowds = nullptr;
	TArray<float>* startTimeAnimationCrowds = nullptr;
	TArray<float>* previousAnimationFrameCrowds = nullptr;
	TArray<EFluxPrimeCrowdAnimationNotify>* crowdsRequestAnimationNotify = nullptr;
	TArray<FVector>* crowdsCurrentTarget = nullptr;
	uint16* memberActive = nullptr;
	
	UPROPERTY(EditAnywhere)
	FFluxPrimeSpatialGridSystemsContext contextSpatialGrid;
};

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	bool IsDebug = false;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* CrowdsComponents = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
	UPROPERTY()
	TArray<FFluxPrimeRuntimeAnimationMapping> CrowdsRuntimeAnimationMapping;
	
	UPROPERTY()
	TArray<FFluxPrimeRuntimeAnimationNotify> CrowdsRuntimeAnimationNotify;
	
	TArray<int16>* InstanceIndexCrowds = nullptr;
	TArray<int8>* CrowdsType = nullptr;
	TArray<int32>* CrowdsCellID = nullptr;
	TArray<EFluxPrimeCrowdState>* CrowdsState = nullptr;
	TArray<EFluxPrimeCrowdState>* CrowdsAnimationState = nullptr;
	TArray<float>* CrowdsStartTimeAnimation = nullptr;
	TArray<float>* CrowdsPreviousAnimationFrame = nullptr;
	TArray<EFluxPrimeCrowdAnimationNotify>* CrowdsRequestAnimationNotify = nullptr;
	TArray<FVector>* CrowdsCurrentTarget = nullptr;
	uint16* MemberActive = nullptr;
	
	FFluxPrimeSpatialGridSystem SpatialGridSystems;
	
private:
	void ShowDebug(FFluxPrimeCrowds& members, int32 indexMembers, int32 animationIndex, float currentFrame)
	{
		FVector textLocation = members.CrowdsLocation[indexMembers] + (FVector::UpVector * FluxConfig::DebugLocationAnimation);
		FString debugData = FString::Printf(TEXT("Animation Index: %d \n Animation Frame: %f"), animationIndex, currentFrame);
		
		DrawDebugString(
			World,
			textLocation,
			debugData,
			nullptr,
			FColor::Yellow,
			0.0f,
			false,
			FluxConfig::DebugScaleFont
		);
	}
	
	void ShowDebug(FString message, FVector location)
	{
		DrawDebugString(
			World,
			location,
			message,
			nullptr,
			FColor::Yellow,
			0.0f,
			false,
			FluxConfig::DebugScaleFont
		);
	}
	
	void InitializeAnimationMapping(const uint16& crowdsCountCatalog, TArray<FName>& crowdsCatalogName, TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>>& crowdsAnimationSoftRef)
	{
		CrowdsRuntimeAnimationMapping.SetNumUninitialized(crowdsCountCatalog);
		CrowdsRuntimeAnimationNotify.SetNumUninitialized(crowdsCountCatalog);
		
		FFluxPrimeRuntimeAnimationMapping animationRuntimeData = FFluxPrimeRuntimeAnimationMapping();
		FFluxPrimeRuntimeAnimationNotify animationRuntimeNotify = FFluxPrimeRuntimeAnimationNotify();
		
		for (int i = 0; i < crowdsCountCatalog; ++i)
		{
			TSoftObjectPtr<UFluxPrimeAnimationData> loadedData = crowdsAnimationSoftRef[crowdsCatalogName[i]];
			UFluxPrimeAnimationData& dataAnim = *loadedData;
				
			for (int k = 0; k < dataAnim.DataAnimations.Num(); ++k)
			{
				int32 index = static_cast<uint8>(dataAnim.DataAnimations[k].AnimationState);
				for (int32 l = 0; l < FluxConfig::AnimationArrayCount; ++l)
				{
					if (dataAnim.DataAnimations[k].AnimationNotifies[l].AnimationNotifyType == EFluxPrimeCrowdAnimationNotify::NotifyNone) continue;
					int32 notifyIndex = animationRuntimeNotify.RuntimeAnimationNotifyData[index].AnimationNotifyCount;
					animationRuntimeNotify.RuntimeAnimationNotifyData[index].AnimationNotify[notifyIndex].AnimationNotifyType = dataAnim.DataAnimations[k].AnimationNotifies[l].AnimationNotifyType;
					animationRuntimeNotify.RuntimeAnimationNotifyData[index].AnimationNotify[notifyIndex].AnimationNotifyFrame = dataAnim.DataAnimations[k].AnimationNotifies[l].AnimationNotifyFrame;
					animationRuntimeNotify.RuntimeAnimationNotifyData[index].AnimationNotifyCount++;
				}
				
				Algo::Sort(MakeArrayView(animationRuntimeNotify.RuntimeAnimationNotifyData[index].AnimationNotify, animationRuntimeNotify.RuntimeAnimationNotifyData[index].AnimationNotifyCount),
				[](const FFluxCrowdsAnimationNotify& A, const FFluxCrowdsAnimationNotify& B)
				{
					return A.AnimationNotifyFrame < B.AnimationNotifyFrame;
				});
				
				animationRuntimeData.AnimationStart[index] = dataAnim.DataAnimations[k].AnimationStartFrame;
				animationRuntimeData.AnimationEnd[index] = dataAnim.DataAnimations[k].AnimationEndFrame;
				animationRuntimeData.AnimationForceNotify[index] = dataAnim.DataAnimations[k].AnimationForceNotify;
			}

			CrowdsRuntimeAnimationMapping[i] = animationRuntimeData;
			CrowdsRuntimeAnimationNotify[i] = animationRuntimeNotify;
		}
	}
	
	bool IsNotifyTriggered(float PrevFrame, float CurrentFrame, float NotifyFrame)
	{
		if (PrevFrame < 0.0f) return false;
		
		if (CurrentFrame >= PrevFrame)
		{
			return NotifyFrame > PrevFrame && NotifyFrame <= CurrentFrame;
		}

		return NotifyFrame > PrevFrame || NotifyFrame <= CurrentFrame;
	}
	
	void PlayAnimation(EFluxPrimeCrowdState& crowdAnimationState, float& startTimeAnimation, 
		int8& crowdType, float& previousFrameAnimation, uint32 indexMember, float realTime)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_PlayAnimation);
		
		uint8 indexAnimation = static_cast<uint8>(crowdAnimationState);
		
		float startFrame = CrowdsRuntimeAnimationMapping[crowdType].AnimationStart[indexAnimation];
		float endFrame = CrowdsRuntimeAnimationMapping[crowdType].AnimationEnd[indexAnimation];
		
		float localAnimTime = realTime - startTimeAnimation;
		float realTimeFrames = localAnimTime * 30.0f;
		float reminder = FMath::Fmod(realTimeFrames, (endFrame - startFrame) + 1.0f);
		float current = startFrame + reminder;
		
		if (CrowdsRuntimeAnimationNotify[crowdType].RuntimeAnimationNotifyData[indexAnimation].AnimationNotifyCount > 0) PlayAnimationNotify(CrowdsRuntimeAnimationNotify[crowdType], indexMember, indexAnimation,current, previousFrameAnimation);
		previousFrameAnimation = current;
	}
	
	void PlayAnimationNotify(FFluxPrimeRuntimeAnimationNotify& notify, uint32 memberIndex, uint8 indexAnimation,float currentFrame, float previousFrame)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_PlayNotifyAnimation);
		
		for (int i = 0; i < notify.RuntimeAnimationNotifyData[indexAnimation].AnimationNotifyCount; ++i)
		{	
			if (!IsNotifyTriggered(previousFrame, currentFrame, notify.RuntimeAnimationNotifyData[indexAnimation].AnimationNotify[i].AnimationNotifyFrame)) continue;

			(*CrowdsRequestAnimationNotify)[memberIndex] = notify.RuntimeAnimationNotifyData[indexAnimation].AnimationNotify[i].AnimationNotifyType;
		}
	}
	
	void UpdateAnimation(int16& instanceIndexCrowd, int8& crowdType, EFluxPrimeCrowdState& crowdState, 
		EFluxPrimeCrowdState& crowdAnimationState, float& startTimeAnimation, float& previousFrameAnimation, float realTime)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_UpdateAnimation);
		
		if (!World) return;
		
		if (crowdAnimationState != crowdState)
		{
			// previous
			uint8 indexAnimation = static_cast<uint8>(crowdAnimationState);
			uint16 previousStartFrame = CrowdsRuntimeAnimationMapping[crowdType].AnimationStart[indexAnimation];
			uint16 previousEndFrame = CrowdsRuntimeAnimationMapping[crowdType].AnimationEnd[indexAnimation];
			uint16 previousTime = startTimeAnimation;
			
			// change current state
			crowdAnimationState = crowdState;
			
			// current
			indexAnimation = static_cast<uint8>(crowdAnimationState);
			uint16 currentStartFrame = CrowdsRuntimeAnimationMapping[crowdType].AnimationStart[indexAnimation];
			uint16 currentEndFrame = CrowdsRuntimeAnimationMapping[crowdType].AnimationEnd[indexAnimation];
			
			SwitchAnimation(instanceIndexCrowd, crowdType, currentStartFrame, currentEndFrame, previousStartFrame, previousEndFrame, previousTime);
			
			// update start time
			startTimeAnimation = realTime;
			previousFrameAnimation = -1.0f;
		}
	}
	
	void SwitchAnimation(uint16 instanceIndexCrowd, int8 type, uint16 currentStartFrame, uint16 currentEndFrame, uint16 previousStartFrame, uint16 previousEndFrame,uint16 previousStartTime)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_Switch);
		
		check(CrowdsComponents->IsValidIndex(type));
	
		// previous
		(*CrowdsComponents)[type]->SetCustomDataValue(
			instanceIndexCrowd,
			0,
			previousStartTime,
			false
			);
	
		(*CrowdsComponents)[type]->SetCustomDataValue(
			instanceIndexCrowd,
			1,
			previousStartFrame,
			false
			);
	
		(*CrowdsComponents)[type]->SetCustomDataValue(
			instanceIndexCrowd,
			2,
			previousEndFrame,
			false
			);
		
		// current
		(*CrowdsComponents)[type]->SetCustomDataValue(
			instanceIndexCrowd,
			3,
			World->GetRealTimeSeconds(),
			false
			);
	
		(*CrowdsComponents)[type]->SetCustomDataValue(
			instanceIndexCrowd,
			4,
			currentStartFrame,
			false
			);
	
		(*CrowdsComponents)[type]->SetCustomDataValue(
			instanceIndexCrowd,
			5,
			currentEndFrame,
			false
			);
	}
	
public:
	void InitializedAnimationSystems(FFluxPrimeAnimationSystemsContext context)
	{
		check(context.world);
		check(context.memberActive);
		check(context.instanceIndexCrowds);
		check(context.typeCrowds);
		check(context.crowdsCellID);
		check(context.stateCrowds);
		check(context.animationStateCrowds);
		check(context.startTimeAnimationCrowds);
		check(context.previousAnimationFrameCrowds);
		check(context.crowdsRequestAnimationNotify);
		check(context.crowdsComponents);
		check(context.crowdsCatalogName);
		check(context.crowdsAnimationSoftRef);
		check(context.crowdsCurrentTarget);
		
		World = context.world;
		IsDebug = context.isDebug;
		InstanceIndexCrowds = context.instanceIndexCrowds;
		CrowdsType = context.typeCrowds;
		CrowdsCellID = context.crowdsCellID;
		CrowdsState = context.stateCrowds;
		CrowdsAnimationState = context.animationStateCrowds;
		CrowdsStartTimeAnimation = context.startTimeAnimationCrowds;
		CrowdsPreviousAnimationFrame = context.previousAnimationFrameCrowds;
		CrowdsRequestAnimationNotify = context.crowdsRequestAnimationNotify;
		MemberActive = context.memberActive;
		CrowdsComponents = context.crowdsComponents;
		CrowdsCurrentTarget = context.crowdsCurrentTarget;
		
		InitializeAnimationMapping(context.crowdsCountCatalog, *context.crowdsCatalogName, *context.crowdsAnimationSoftRef);
		
		SpatialGridSystems.InitializedSpatialGridSystems(context.contextSpatialGrid);
		SpatialGridSystems.BakeSpatialGridSystems();
	}
	
	void UpdateAnimationSystemsFrame()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_Update);
		
		SpatialGridSystems.UpdateSpatialGridSystem();
		
		auto& instanceIndexCrowds = *InstanceIndexCrowds;
		auto& typeCrowds = *CrowdsType;
		auto& stateCrowds = *CrowdsState;
		auto& animStateCrowds = *CrowdsAnimationState;
		auto& startTimeAnimCrowds = *CrowdsStartTimeAnimation;
		auto& previousFrameAnimCrowds = *CrowdsPreviousAnimationFrame;
		auto& cellId = *CrowdsCellID;
		auto& targetLocation = *CrowdsCurrentTarget;
		float realTime = World->GetRealTimeSeconds();
		
		for (int i = 0; i < *MemberActive; ++i)
		{
			UpdateAnimation(instanceIndexCrowds[i], typeCrowds[i], stateCrowds[i], 
				animStateCrowds[i], startTimeAnimCrowds[i], previousFrameAnimCrowds[i], realTime);
			
			int32 targetCellId = SpatialGridSystems.GetSpatialGridSystemsCellID(targetLocation[i]);
			if (cellId[i] != targetCellId && !CrowdsRuntimeAnimationMapping[typeCrowds[i]].AnimationForceNotify) continue;
			
			PlayAnimation(animStateCrowds[i], startTimeAnimCrowds[i], typeCrowds[i],
				previousFrameAnimCrowds[i], i, realTime);
		}
	}
	
	void EndPlayAnimationSystems()
	{
		
	}
};