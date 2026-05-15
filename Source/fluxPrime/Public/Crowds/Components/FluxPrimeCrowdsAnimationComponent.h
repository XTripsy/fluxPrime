// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FluxPrimeCrowdsAnimationComponent.generated.h"


struct FFluxPrimeAnimationSystems;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLUXPRIME_API UFluxPrimeCrowdsAnimationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void Initialize(FFluxPrimeAnimationSystems& animationSystems);
	
	UFUNCTION()
	void OnAnimationAttackNotify(int32 id);
	
	UFUNCTION()
	void OnAnimationVFXNotify(int32 id);
	
	UFUNCTION()
	void OnAnimationSFXNotify(int32 id);
};
