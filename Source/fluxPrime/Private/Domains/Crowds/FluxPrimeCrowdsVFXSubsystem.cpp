#include "Domains/Crowds/FluxPrimeCrowdsVFXSubsystem.h"

#include "Engine/World.h"
#include "Framework/Core/FluxPrimePauseWorldSubsystem.h"
#include "NiagaraComponent.h"
#include "NiagaraDataChannel.h"
#include "NiagaraDataChannelAccessor.h"
#include "NiagaraDataChannel_Global.h"
#include "NiagaraSystem.h"

void UFluxPrimeCrowdsVFXSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency<UFluxPrimePauseWorldSubsystem>();
	Super::Initialize(Collection);
}

bool UFluxPrimeCrowdsVFXSubsystem::IsConfigurationValid(UNiagaraSystem* System,
	UNiagaraDataChannelAsset* Channel, FName PositionVariable)
{
	if (!IsValid(System) || !IsValid(Channel) || !IsValid(Channel->Get()) || PositionVariable.IsNone()) return false;
	// An origin listener cannot resolve arbitrary spatial islands.
	if (!Channel->Get()->IsA<UNiagaraDataChannel_Global>()) return false;
	int32 Matches = 0;
	for (const FNiagaraDataChannelVariable& Variable : Channel->Get()->GetVariables())
	{
		if (Variable.GetName() != PositionVariable) continue;
		if (Variable.GetType() != FNiagaraTypeDefinition::GetPositionDef()) return false;
		++Matches;
	}
	return Matches == 1;
}

UNiagaraComponent* UFluxPrimeCrowdsVFXSubsystem::FindListener(const UNiagaraDataChannelAsset* Channel) const
{
	for (const auto& Listener : Listeners)
	{
		if (Listener.Channel == Channel && IsValid(Listener.Component)) return Listener.Component;
	}
	return nullptr;
}

bool UFluxPrimeCrowdsVFXSubsystem::Publish(UNiagaraSystem* System, UNiagaraDataChannelAsset* Channel,
	FName PositionVariable, const FVector& Position)
{
	check(IsInGameThread());
	UWorld* World = GetWorld();
	if (bShuttingDown || !IsValid(World) || World->bIsTearingDown || World->GetNetMode() == NM_DedicatedServer
		|| UFluxPrimePauseWorldSubsystem::IsWorldPaused(World) || Position.ContainsNaN()
		|| !IsConfigurationValid(System, Channel, PositionVariable)) return false;

	FFluxPrimeCrowdsVFXListener* Entry = Listeners.FindByPredicate(
		[Channel](const auto& Listener) { return Listener.Channel == Channel; });
	if (Entry && IsValid(Entry->Component)
		&& (Entry->Component->GetAsset() != System || Entry->PositionVariable != PositionVariable)) return false;

	if (!Entry)
	{
		Entry = &Listeners.AddDefaulted_GetRef();
		Entry->Channel = Channel;
	}
	UNiagaraComponent* Component = Entry->Component;
	if (!IsValid(Component))
	{
		Component = NewObject<UNiagaraComponent>(World, NAME_None, RF_Transient);
		Entry->Component = Component;
		Entry->PositionVariable = PositionVariable;
		Component->SetAsset(System);
		Component->SetAutoDestroy(false);
		Component->SetAutoActivate(false);
		Component->SetTickBehavior(ENiagaraTickBehavior::ForceTickLast);
		Component->RegisterComponentWithWorld(World);
		if (!Component->IsRegistered())
		{
			Component->DestroyComponent();
			Entry->Component = nullptr;
			return false;
		}
		Component->Activate(true);
	}
	else
	{
		if (!Component->IsRegistered()) return false;
		// Do not reset live particles when subsequent notifies arrive.
		if (Component->IsComplete()) Component->Activate(true);
	}
	if (auto* Pause = World->GetSubsystem<UFluxPrimePauseWorldSubsystem>()) Pause->RegisterNiagaraComponent(Component);
	if (Component->IsPaused()) return false;

	UNiagaraDataChannelWriter* Writer = UNiagaraDataChannelLibrary::WriteToNiagaraDataChannel(
		World, Channel, FNiagaraDataChannelSearchParameters(Position), 1,
		false, true, true, TEXT("FluxPrime.NotifySpawnVFX"));
	if (!Writer) return false;
	Writer->WritePosition(PositionVariable, 0, Position);
	return true;
}

void UFluxPrimeCrowdsVFXSubsystem::ShutdownListeners()
{
	check(IsInGameThread());
	for (auto& Listener : Listeners)
	{
		if (auto* Pause = GetWorld() ? GetWorld()->GetSubsystem<UFluxPrimePauseWorldSubsystem>() : nullptr)
			Pause->UnregisterNiagaraComponent(Listener.Component);
		if (IsValid(Listener.Component)) Listener.Component->DestroyComponent();
	}
	Listeners.Reset();
}

void UFluxPrimeCrowdsVFXSubsystem::Deinitialize()
{
	bShuttingDown = true;
	ShutdownListeners();
	Super::Deinitialize();
}
