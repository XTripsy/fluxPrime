// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/Components/FluxPrimeTargetComponent.h"


UFluxPrimeTargetComponent::UFluxPrimeTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

const uint16 UFluxPrimeTargetComponent::GetPriorityTarget()
{
	return PriorityTarget;
}

void UFluxPrimeTargetComponent::UpdatePriorityTarget(int32 newPriorityTarget)
{
	PriorityTarget = newPriorityTarget;
}
