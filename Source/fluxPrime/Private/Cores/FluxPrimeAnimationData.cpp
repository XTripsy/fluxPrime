// Fill out your copyright notice in the Description page of Project Settings.


#include "Cores/FluxPrimeAnimationData.h"

#if WITH_EDITOR
void UFluxPrimeAnimationData::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	const FName MemberPropertyName = (PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;
	
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UFluxPrimeAnimationData, DataAnimations) ||
		MemberPropertyName == GET_MEMBER_NAME_CHECKED(UFluxPrimeAnimationData, Animations))
	{
		for (auto& pair : DataAnimations)
		{
			int32 index = pair.AnimationIndex;
		
			if (!Animations.IsValidIndex(index))
			{
				pair.AnimationStartFrame = 0;
				pair.AnimationEndFrame = 0;
				continue;
			}
		
			pair.AnimationStartFrame = Animations[index].StartFrame;
			pair.AnimationEndFrame = Animations[index].EndFrame;
		}
	}
	
}
#endif
