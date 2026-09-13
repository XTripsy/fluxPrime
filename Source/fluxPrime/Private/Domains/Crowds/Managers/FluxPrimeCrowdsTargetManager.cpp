// Fill out your copyright notice in the Description page of Project Settings.


#include "Domains/Crowds/Managers/FluxPrimeCrowdsTargetManager.h"


UFluxPrimeCrowdsTargetManager::UFluxPrimeCrowdsTargetManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

const uint16 UFluxPrimeCrowdsTargetManager::GetPriorityTarget()
{
	return PriorityTarget;
}

void UFluxPrimeCrowdsTargetManager::UpdatePriorityTarget(int32 newPriorityTarget)
{
	PriorityTarget = newPriorityTarget;
}
