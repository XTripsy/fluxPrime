#include "Framework/Core/FluxPrimePauseWorldSubsystem.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"

bool UFluxPrimePauseWorldSubsystem::IsWorldPaused(const UWorld* World)
{
	return World && World->IsPaused();
}

float UFluxPrimePauseWorldSubsystem::GetSimulationTime(const UWorld* World)
{
	return World ? World->GetTimeSeconds() : 0.0f;
}

void UFluxPrimePauseWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	bWasPaused = IsWorldPaused(GetWorld());
	FWorldDelegates::OnWorldTickStart.AddUObject(this, &ThisClass::OnWorldTickStart);
}

void UFluxPrimePauseWorldSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldTickStart.RemoveAll(this);
	for (FPresentation& Entry : Presentation)
	{
		if (UNiagaraComponent* Component = Entry.Component.Get())
		{
			Component->OnSystemFinished.RemoveDynamic(this, &ThisClass::OnNiagaraFinished);
		}
	}
	Presentation.Reset();
	OnResume.Clear();
	PendingOwners.Reset();
	Super::Deinitialize();
}

void UFluxPrimePauseWorldSubsystem::ApplyPause(FPresentation& Entry, bool bPaused)
{
	UNiagaraComponent* Component = Entry.Component.Get();
	if (!Component) return;
	if (bPaused)
	{
		if (!Component->IsPaused())
		{
			Entry.bImposedPause = true;
			Component->SetPaused(true);
		}
	}
	else if (Entry.bImposedPause)
	{
		Component->SetPaused(false);
		Entry.bImposedPause = false;
	}
}

void UFluxPrimePauseWorldSubsystem::RegisterNiagaraComponent(UNiagaraComponent* Component)
{
	check(IsInGameThread());
	if (!IsValid(Component) || Component->GetWorld() != GetWorld()) return;
	if (Registered.Contains(Component)) return;
	for (int32 Index = Presentation.Num() - 1; Index >= 0; --Index)
	{
		if (!Presentation[Index].Component.IsValid())
		{
			Registered.Remove(Presentation[Index].Component);
			Presentation.RemoveAtSwap(Index);
		}
	}
	Registered.Add(Component);
	FPresentation& Entry = Presentation.AddDefaulted_GetRef();
	Entry.Component = Component;
	Component->OnSystemFinished.AddUniqueDynamic(this, &ThisClass::OnNiagaraFinished);
	ApplyPause(Entry, IsWorldPaused(GetWorld()));
}

void UFluxPrimePauseWorldSubsystem::UnregisterNiagaraComponent(UNiagaraComponent* Component)
{
	check(IsInGameThread());
	if (!Component) return;
	for (int32 Index = Presentation.Num() - 1; Index >= 0; --Index)
	{
		if (Presentation[Index].Component == Component || !Presentation[Index].Component.IsValid())
		{
			ApplyPause(Presentation[Index], false);
			Registered.Remove(Presentation[Index].Component);
			Presentation.RemoveAtSwap(Index);
		}
	}
	if (IsValid(Component)) Component->OnSystemFinished.RemoveDynamic(this, &ThisClass::OnNiagaraFinished);
}

void UFluxPrimePauseWorldSubsystem::OnNiagaraFinished(UNiagaraComponent* Component)
{
	UnregisterNiagaraComponent(Component);
}

void UFluxPrimePauseWorldSubsystem::OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaSeconds)
{
	if (World != GetWorld()) return;
	const bool bPaused = IsWorldPaused(World);
	if (bPaused == bWasPaused)
	{
		DrainPendingWork();
		return;
	}
	bWasPaused = bPaused;
	for (int32 Index = Presentation.Num() - 1; Index >= 0; --Index)
	{
		if (!Presentation[Index].Component.IsValid())
		{
			Registered.Remove(Presentation[Index].Component);
			Presentation.RemoveAtSwap(Index);
		}
		else ApplyPause(Presentation[Index], bPaused);
	}
	DrainPendingWork();
}

void UFluxPrimePauseWorldSubsystem::DrainPendingWork()
{
	if (bDraining || PendingOwners.IsEmpty() || IsWorldPaused(GetWorld())) return;
	TGuardValue<bool> Guard(bDraining, true);
	OnResume.Broadcast();
}
