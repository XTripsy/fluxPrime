// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Net/FluxPrimeNetManager.h"
#include "StructUtils/InstancedStruct.h"

UFluxPrimeNetManager::UFluxPrimeNetManager()
{
	SetIsReplicatedByDefault(true);
}

void UFluxPrimeNetManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnCrowdsNetManagerActionChange.Unbind();
	
	Super::EndPlay(EndPlayReason);
}

void UFluxPrimeNetManager::ServerActionChange_Implementation(const FInstancedStruct& payload)
{
	OnCrowdsNetManagerActionChange.ExecuteIfBound(payload);
	MulticastActionChange(payload);
}

void UFluxPrimeNetManager::MulticastActionChange_Implementation(const FInstancedStruct& payload)
{
	OnCrowdsNetManagerActionChange.ExecuteIfBound(payload);
}

void UFluxPrimeNetManager::OnActionChange(FInstancedStruct payload)
{
	if (!GetOwner()->HasAuthority())
	{
		ServerActionChange(payload);
		return;
	}
	
	MulticastActionChange(payload);
}
