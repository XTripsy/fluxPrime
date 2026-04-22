// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimToTextureDataAsset.h"
#include "FluxPrimeStruct.h"
#include "FluxPrimeAnimationData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class FLUXPRIME_API UFluxPrimeAnimationData : public UAnimToTextureDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<bool> AnimationLoops;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Notify")
	TArray<FFluxCrowdsAnimationNotify> AnimationNotifies;
};
