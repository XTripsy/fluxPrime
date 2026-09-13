// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimToTextureDataAsset.h"
#include "Containers/StaticArray.h"
#include "Generics/Data/FluxPrimeAnimationData.h"
#include "FluxPrimeAnimationIdentity.generated.h"

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
