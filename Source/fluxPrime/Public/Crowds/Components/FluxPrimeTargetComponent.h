// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FluxPrimeTargetComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLUXPRIME_API UFluxPrimeTargetComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "Priority Target", meta = (AllowPrivateAccess = true))
	uint16 PriorityTarget = 0;
	
public:
	UFluxPrimeTargetComponent();
	const uint16 GetPriorityTarget();
	UFUNCTION(BlueprintCallable)
	void UpdatePriorityTarget(int32 newPriorityTarget);
};
