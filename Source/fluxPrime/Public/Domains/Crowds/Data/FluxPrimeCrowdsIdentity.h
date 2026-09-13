// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Framework/Data/FluxPrimeBaseIdentity.h"
#include "Framework/Data/FluxPrimeStateData.h"
#include "Generics/Data/FluxPrimeAbilityData.h"
#include "FluxPrimeCrowdsIdentity.generated.h"

class UFluxPrimeAnimationData;

UCLASS(BlueprintType)
class FLUXPRIME_API UFluxPrimeCrowdsIdentity : public UFluxPrimeBaseIdentity
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UFluxPrimeAnimationData> AnimationData;
	
	UPROPERTY(EditAnywhere)
	int32 Speed;
	
	UPROPERTY(EditAnywhere)
	int32 AbilityRange;
	
	UPROPERTY(EditAnywhere)
	float AggroDistance;
	
	UPROPERTY(EditAnywhere)
	FAbilityFragment AbilityFragment;
	
	UPROPERTY(EditAnywhere)
	float Damage;
	
	UPROPERTY(EditAnywhere)
	int32 Health;
	
};
