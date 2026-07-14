#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeSpatialGridSystems.h"
#include "DrawDebugHelpers.h"
#include "FluxPrimeBaseSystems.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeAnimationSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool isDebug = false;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* crowdsComponents = nullptr;
	TArray<FFluxPrimeCrowdsCatalog>* crowdsCatalog = nullptr;
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>>* crowdsAnimationSoftRef = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	uint32 totalMember = 0;
	
	TArray<int16>* idCrowds = nullptr;
	TArray<int8>* typeCrowds = nullptr;
	TArray<int32>* crowdsCellID = nullptr;
	TArray<EFluxPrimeCrowdState>* stateCrowds = nullptr;
	TArray<EFluxPrimeCrowdState>* animationStateCrowds = nullptr;
	TArray<float>* startTimeAnimationCrowds = nullptr;
	TArray<float>* previousAnimationFrameCrowds = nullptr;
	TArray<EFluxPrimeCrowdAnimationNotify>* crowdsRequestAnimationNotify = nullptr;
	TArray<FVector>* crowdsCurrentTargetLocationPath = nullptr;
	uint16* memberActive = nullptr;
	
	UPROPERTY(EditAnywhere)
	FFluxPrimeSpatialGridSystemsContext contextSpatialGrid;
};

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationSystems : public FFluxPrimeBaseSystems
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
	
	TArray<int16>* CrowdsID = nullptr;
	TArray<int8>* CrowdsType = nullptr;
	TArray<int32>* CrowdsCellID = nullptr;
	TArray<EFluxPrimeCrowdState>* CrowdsState = nullptr;
	TArray<EFluxPrimeCrowdState>* CrowdsAnimationState = nullptr;
	TArray<float>* CrowdsStartTimeAnimation = nullptr;
	TArray<float>* CrowdsPreviousAnimationFrame = nullptr;
	TArray<EFluxPrimeCrowdAnimationNotify>* CrowdsRequestAnimationNotify = nullptr;
	TArray<FVector>* CrowdsCurrentTargetLocationPath = nullptr;
	uint16* MemberActive = nullptr;
	
	FFluxPrimeSpatialGridSystems SpatialGridSystems;
	
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
	
	void InitializeAnimationMapping(TArray<FFluxPrimeCrowdsCatalog>& crowdsCatalog, TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>>& crowdsAnimationSoftRef)
	{
		CrowdsRuntimeAnimationMapping.SetNumUninitialized(crowdsCatalog.Num());
		CrowdsRuntimeAnimationNotify.SetNumUninitialized(crowdsCatalog.Num());
		
		FFluxPrimeRuntimeAnimationMapping animationRuntimeData = FFluxPrimeRuntimeAnimationMapping();
		FFluxPrimeRuntimeAnimationNotify animationRuntimeNotify = FFluxPrimeRuntimeAnimationNotify();
		
		for (int i = 0; i < crowdsCatalog.Num(); ++i)
		{
			TSoftObjectPtr<UFluxPrimeAnimationData> loadedData = crowdsAnimationSoftRef[crowdsCatalog[i].CrowdsIdentity->Identity];
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
				animationRuntimeData.AnimationLoop[index] = dataAnim.DataAnimations[k].AnimationLoops;
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
	
	void PlayAnimation(TArray<EFluxPrimeCrowdState>& crowdsAnimationState, TArray<float>& startTimeAnimation, 
		TArray<int8>& crowdsType, TArray<float>& previousFrameAnimation, uint16 indexMembers, float realTime)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_PlayAnimation);
		
		uint8 type = crowdsType[indexMembers];
		uint8 indexAnimation = static_cast<uint8>(crowdsAnimationState[indexMembers]);
		
		float startFrame = CrowdsRuntimeAnimationMapping[type].AnimationStart[indexAnimation];
		float endFrame = CrowdsRuntimeAnimationMapping[type].AnimationEnd[indexAnimation];
		
		float localAnimTime = realTime - startTimeAnimation[indexMembers];
		float realTimeFrames = localAnimTime * 30.0f;
		float reminder = FMath::Fmod(realTimeFrames, (endFrame - startFrame) + 1.0f);
		float current = startFrame + reminder;
		
		//if (IsDebug) ShowDebug(members, indexMembers, indexAnimation, current);
		
		if (CrowdsRuntimeAnimationNotify[type].RuntimeAnimationNotifyData[indexAnimation].AnimationNotifyCount > 0) PlayAnimationNotify(CrowdsRuntimeAnimationNotify[type], indexMembers, indexAnimation,current, previousFrameAnimation[indexMembers]);
		previousFrameAnimation[indexMembers] = current;
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
	
	void UpdateAnimation(TArray<int16>& crowdsID, TArray<int8>& crowdsType, TArray<EFluxPrimeCrowdState>& crowdsState, 
		TArray<EFluxPrimeCrowdState>& crowdsAnimationState, TArray<float>& startTimeAnimation, TArray<float>& previousFrameAnimation, uint16 i, float realTime)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_UpdateAnimation);
		
		if (!World) return;
		
		if (crowdsAnimationState[i] != crowdsState[i])
		{
			// identity
			uint8 id = crowdsID[i];
			uint8 type = crowdsType[i];
			
			// previous
			uint8 indexAnimation = static_cast<uint8>(crowdsAnimationState[i]);
			uint16 previousStartFrame = CrowdsRuntimeAnimationMapping[type].AnimationStart[indexAnimation];
			uint16 previousEndFrame = CrowdsRuntimeAnimationMapping[type].AnimationEnd[indexAnimation];
			uint16 previousTime = startTimeAnimation[i];
			
			// change current state
			crowdsAnimationState[i] = crowdsState[i];
			
			// current
			indexAnimation = static_cast<uint8>(crowdsAnimationState[i]);
			uint16 currentStartFrame = CrowdsRuntimeAnimationMapping[type].AnimationStart[indexAnimation];
			uint16 currentEndFrame = CrowdsRuntimeAnimationMapping[type].AnimationEnd[indexAnimation];
			
			SwitchAnimation(id, type, currentStartFrame, currentEndFrame, previousStartFrame, previousEndFrame, previousTime);
			
			// update start time
			startTimeAnimation[i] = realTime;
			previousFrameAnimation[i] = -1.0f;
		}
	}
	
	void SwitchAnimation(uint16 id, int8 type,	uint16 currentStartFrame, uint16 currentEndFrame, uint16 previousStartFrame, uint16 previousEndFrame,uint16 previousStartTime)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_Switch);
		
		if (!CrowdsComponents->IsValidIndex(type)) return;
	
		// previous
		(*CrowdsComponents)[type]->SetCustomDataValue(
			id,
			0,
			previousStartTime,
			false
			);
	
		(*CrowdsComponents)[type]->SetCustomDataValue(
			id,
			1,
			previousStartFrame,
			false
			);
	
		(*CrowdsComponents)[type]->SetCustomDataValue(
			id,
			2,
			previousEndFrame,
			false
			);
		
		// current
		(*CrowdsComponents)[type]->SetCustomDataValue(
			id,
			3,
			World->GetRealTimeSeconds(),
			false
			);
	
		(*CrowdsComponents)[type]->SetCustomDataValue(
			id,
			4,
			currentStartFrame,
			false
			);
	
		(*CrowdsComponents)[type]->SetCustomDataValue(
			id,
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
		check(context.idCrowds);
		check(context.typeCrowds);
		check(context.crowdsCellID);
		check(context.stateCrowds);
		check(context.animationStateCrowds);
		check(context.startTimeAnimationCrowds);
		check(context.previousAnimationFrameCrowds);
		check(context.crowdsRequestAnimationNotify);
		check(context.crowdsComponents);
		check(context.crowdsCatalog);
		check(context.crowdsAnimationSoftRef);
		check(context.crowdsCurrentTargetLocationPath);
		
		World = context.world;
		IsDebug = context.isDebug;
		CrowdsID = context.idCrowds;
		CrowdsType = context.typeCrowds;
		CrowdsCellID = context.crowdsCellID;
		CrowdsState = context.stateCrowds;
		CrowdsAnimationState = context.animationStateCrowds;
		CrowdsStartTimeAnimation = context.startTimeAnimationCrowds;
		CrowdsPreviousAnimationFrame = context.previousAnimationFrameCrowds;
		CrowdsRequestAnimationNotify = context.crowdsRequestAnimationNotify;
		MemberActive = context.memberActive;
		CrowdsComponents = context.crowdsComponents;
		CrowdsCurrentTargetLocationPath = context.crowdsCurrentTargetLocationPath;
		
		InitializeAnimationMapping(*context.crowdsCatalog, *context.crowdsAnimationSoftRef);
		
		SpatialGridSystems.InitializedSpatialGridSystems(context.contextSpatialGrid);
		SpatialGridSystems.BakeSpatialGridSystems();
	}
	
	void UpdateAnimationSystemsFrame()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_Update);
		
		SpatialGridSystems.UpdateSpatialGridSystem();
		
		auto& idCrowds = *CrowdsID;
		auto& typeCrowds = *CrowdsType;
		auto& stateCrowds = *CrowdsState;
		auto& animStateCrowds = *CrowdsAnimationState;
		auto& startTimeAnimCrowds = *CrowdsStartTimeAnimation;
		auto& previousFrameAnimCrowds = *CrowdsPreviousAnimationFrame;
		auto& cellId = *CrowdsCellID;
		auto& targetLocation = *CrowdsCurrentTargetLocationPath;
		float realTime = World->GetRealTimeSeconds();
		
		for (int i = 0; i < *MemberActive; ++i)
		{
			UpdateAnimation(idCrowds, typeCrowds,stateCrowds, animStateCrowds, startTimeAnimCrowds, previousFrameAnimCrowds, i, realTime);
			
			int32 targetCellId = SpatialGridSystems.GetSpatialGridSystemsCellID(targetLocation[i]);
			if (cellId[i] != targetCellId) continue;
			
			PlayAnimation(animStateCrowds, startTimeAnimCrowds, typeCrowds, previousFrameAnimCrowds, i, realTime);
		}
	}
	
	void EndPlayAnimationSystems()
	{
		
	}
};