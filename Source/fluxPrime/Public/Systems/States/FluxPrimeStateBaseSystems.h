#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeBaseStateSystems.generated.h"

DECLARE_DELEGATE_OneParam(FOnCrowdsStateChange, const FInstancedStruct& payload);

USTRUCT(BlueprintType)
struct FFluxPrimeBaseStateSystems
{
	GENERATED_BODY()
	
public:
	FOnCrowdsStateChange OnCrowdsStateChange;
	
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
private:
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
	
protected:
	void UpdateAnimation(FFluxPrimeCrowds& members, uint16 i)
	{
		if (!World) return;
		
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
			members.CrowdsStartTimeAnimation[i] = World->GetRealTimeSeconds();
		}
	}
};
