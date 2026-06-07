// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimToTextureDataAsset.h"
#include "FluxPrimeStruct.h"
#include "Containers/StaticArray.h"
#include "FluxPrimeAnimationData.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationDefinition
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	EFluxPrimeCrowdState AnimationState;
	
	UPROPERTY(EditAnywhere)
	int8 AnimationIndex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 AnimationStartFrame;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 AnimationEndFrame;
	
	UPROPERTY(EditAnywhere)
	bool AnimationLoops = true;
	
	UPROPERTY(EditAnywhere)
	FFluxCrowdsAnimationNotify AnimationNotifies[FluxConfig::AnimationArrayCount];
};

UCLASS(BlueprintType, Blueprintable)
class FLUXPRIME_API UFluxPrimeAnimationData : public UAnimToTextureDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Animation")
	TArray<FFluxPrimeAnimationDefinition> DataAnimations;
	
protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
};
