// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "Components/ActorComponent.h"
#include "Crowds/Modules/FluxPrimeCrowdsAnimationModule.h"
#include "FluxPrimeCrowdsAnimationComponent.generated.h"

struct FFluxPrimeAnimationSystems;

/*USTRUCT()
struct FFluxPrimeCrowdsAnimationComponentContext
{
	GENERATED_BODY()
	
	FFluxPrimeAnimationSystems* animationSystems;
	
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> crowdsComponents;
};*/

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLUXPRIME_API UFluxPrimeCrowdsAnimationComponent : public UActorComponent
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CrowdsComponents;

public:
	//void Initialize(FFluxPrimeAnimationSystems& animationSystems, TArray<TObjectPtr<UInstancedStaticMeshComponent>> crowdsComponents);
	//void Initialize(FFluxPrimeCrowdsAnimationComponentContext context);
	
	UFUNCTION()
	void OnAnimationAttackNotify(int32 id);
	
	UFUNCTION()
	void OnAnimationVFXNotify(int32 id);
	
	UFUNCTION()
	void OnAnimationSFXNotify(int32 id);
	
	void SwitchAnimation(const FInstancedStruct& payload);
};
