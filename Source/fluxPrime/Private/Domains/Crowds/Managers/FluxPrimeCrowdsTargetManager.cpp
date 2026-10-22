// Fill out your copyright notice in the Description page of Project Settings.


#include "Domains/Crowds/Managers/FluxPrimeCrowdsTargetManager.h"

#include "Domains/Crowds/FluxPrimeCrowdsWorldSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsPayloadData.h"

void UFluxPrimeCrowdsTargetManager::ServerActionChange_Implementation(const FInstancedStruct& payload)
{
	if (payload.GetScriptStruct() == FFluxPrimeCrwodsUpdatePriorityActionPayload::StaticStruct())
	{
		const FFluxPrimeCrwodsUpdatePriorityActionPayload& data = payload.Get<FFluxPrimeCrwodsUpdatePriorityActionPayload>();
		
		PriorityTarget = data.NewPriority;
		
		UFluxPrimeCrowdsWorldSubsystem* crowdSubsystem = GetWorld()->GetSubsystem<UFluxPrimeCrowdsWorldSubsystem>();
		if (!IsValid(crowdSubsystem)) return;
		
		FFluxPrimeCrwodsUpdatePriorityActionPayload crowdsUpdatePriorityActionPayload;
		crowdsUpdatePriorityActionPayload.PlayerID = data.PlayerID;
		crowdsUpdatePriorityActionPayload.NewPriority = PriorityTarget;
	
		crowdSubsystem->RequestAction(FInstancedStruct::Make(crowdsUpdatePriorityActionPayload));
	}
}

UFluxPrimeCrowdsTargetManager::UFluxPrimeCrowdsTargetManager()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

const uint16 UFluxPrimeCrowdsTargetManager::GetPriorityTarget()
{
	return PriorityTarget;
}

void UFluxPrimeCrowdsTargetManager::UpdatePriorityTarget(int32 newPriorityTarget)
{
	PriorityTarget = newPriorityTarget;
	
	APawn* pawn = Cast<APawn>(GetOwner());
	APlayerState* playerState = pawn->GetPlayerState();
	
	FFluxPrimeCrwodsUpdatePriorityActionPayload crowdsUpdatePriorityActionPayload;
	crowdsUpdatePriorityActionPayload.PlayerID = playerState->GetUniqueId();
	crowdsUpdatePriorityActionPayload.NewPriority = PriorityTarget;
	
	ServerActionChange(FInstancedStruct::Make(crowdsUpdatePriorityActionPayload));
}
