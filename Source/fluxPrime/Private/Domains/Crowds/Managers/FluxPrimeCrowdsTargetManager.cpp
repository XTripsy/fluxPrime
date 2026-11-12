// Fill out your copyright notice in the Description page of Project Settings.


#include "Domains/Crowds/Managers/FluxPrimeCrowdsTargetManager.h"

#include "Domains/Crowds/FluxPrimeCrowdsWorldSubsystem.h"
#include "fluxPrime.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsPayloadData.h"

APlayerState* UFluxPrimeCrowdsTargetManager::GetOwningPlayerState() const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		UE_LOG(LogFluxPrime, Warning,
			TEXT("Target manager %s is attached to %s, which is not a pawn; priority target cannot be resolved."),
			*GetName(), GetOwner() ? *GetOwner()->GetName() : TEXT("null owner"));
		return nullptr;
	}

	if (const AController* Controller = Pawn->GetController())
	{
		if (Controller->PlayerState) return Controller->PlayerState;
	}

	if (APlayerState* PlayerState = Pawn->GetPlayerState()) return PlayerState;

	UE_LOG(LogFluxPrime, Warning,
		TEXT("Target manager %s could not resolve a player state for pawn %s (possession may be in progress)."),
		*GetName(), *Pawn->GetName());
	return nullptr;
}

void UFluxPrimeCrowdsTargetManager::ServerActionChange_Implementation(const FInstancedStruct& payload)
{
	if (payload.GetScriptStruct() != FFluxPrimeCrwodsUpdatePriorityActionPayload::StaticStruct()) return;

	const FFluxPrimeCrwodsUpdatePriorityActionPayload& data = payload.Get<FFluxPrimeCrwodsUpdatePriorityActionPayload>();

	if (data.NewPriority == INDEX_NONE)
	{
		UE_LOG(LogFluxPrime, Warning, TEXT("Target manager %s rejected a priority update without a value."), *GetName());
		return;
	}

	// Never trust the client-supplied id: derive it from the connection that owns this component.
	APlayerState* OwningPlayerState = GetOwningPlayerState();
	if (!OwningPlayerState) return;

	const FUniqueNetIdRepl PlayerId = OwningPlayerState->GetUniqueId();
	if (!PlayerId.IsValid())
	{
		UE_LOG(LogFluxPrime, Warning, TEXT("Target manager %s rejected a priority update for an invalid player id."), *GetName());
		return;
	}

	PriorityTarget = static_cast<uint16>(FMath::Clamp(data.NewPriority, 0, static_cast<int32>(MAX_uint16)));

	UWorld* World = GetWorld();
	UFluxPrimeCrowdsWorldSubsystem* crowdSubsystem = World ? World->GetSubsystem<UFluxPrimeCrowdsWorldSubsystem>() : nullptr;
	if (!IsValid(crowdSubsystem))
	{
		UE_LOG(LogFluxPrime, Warning, TEXT("Target manager %s could not dispatch a priority update: crowds subsystem is unavailable."), *GetName());
		return;
	}

	FFluxPrimeCrwodsUpdatePriorityActionPayload crowdsUpdatePriorityActionPayload;
	crowdsUpdatePriorityActionPayload.PlayerID = PlayerId;
	crowdsUpdatePriorityActionPayload.NewPriority = PriorityTarget;

	crowdSubsystem->RequestAction(FInstancedStruct::Make(crowdsUpdatePriorityActionPayload));
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
	PriorityTarget = static_cast<uint16>(FMath::Clamp(newPriorityTarget, 0, static_cast<int32>(MAX_uint16)));

	APlayerState* OwningPlayerState = GetOwningPlayerState();
	if (!OwningPlayerState) return;

	const FUniqueNetIdRepl PlayerId = OwningPlayerState->GetUniqueId();
	if (!PlayerId.IsValid())
	{
		UE_LOG(LogFluxPrime, Warning, TEXT("Target manager %s cannot send a priority update for an invalid player id."), *GetName());
		return;
	}

	FFluxPrimeCrwodsUpdatePriorityActionPayload crowdsUpdatePriorityActionPayload;
	crowdsUpdatePriorityActionPayload.PlayerID = PlayerId;
	crowdsUpdatePriorityActionPayload.NewPriority = PriorityTarget;

	ServerActionChange(FInstancedStruct::Make(crowdsUpdatePriorityActionPayload));
}
