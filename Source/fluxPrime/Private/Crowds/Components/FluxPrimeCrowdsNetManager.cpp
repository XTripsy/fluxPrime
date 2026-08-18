// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/Components/FluxPrimeCrowdsNetManager.h"
#include "StructUtils/InstancedStruct.h"

UFluxPrimeCrowdsNetManager::UFluxPrimeCrowdsNetManager()
{
    SetIsReplicatedByDefault(true);
}

void UFluxPrimeCrowdsNetManager::MulticastActionChange_Implementation(const FInstancedStruct& payload)
{
	OnCrowdsNetManagerActionChange.ExecuteIfBound(payload);
}

void UFluxPrimeCrowdsNetManager::OnActionChange(FInstancedStruct payload)
{
	MulticastActionChange(payload);
}
