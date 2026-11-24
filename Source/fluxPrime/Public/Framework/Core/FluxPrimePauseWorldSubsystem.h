#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseWorldSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "FluxPrimePauseWorldSubsystem.generated.h"

class UNiagaraComponent;

/** World-game pause boundary for presentation that runs independently of manager ticks. */
UCLASS()
class FLUXPRIME_API UFluxPrimePauseWorldSubsystem : public UFluxPrimeBaseWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	static bool IsWorldPaused(const UWorld* World);
	static float GetSimulationTime(const UWorld* World);
	void RegisterNiagaraComponent(UNiagaraComponent* Component);
	void UnregisterNiagaraComponent(UNiagaraComponent* Component);

	/** Runs on the game thread, before resumed actor simulation. */
	FSimpleMulticastDelegate OnResume;
	void DrainPendingWork();
	void RetainPendingOwner(UObject* Owner) { PendingOwners.Add(Owner); }
	void ReleasePendingOwner(UObject* Owner) { PendingOwners.Remove(Owner); }

private:
	UPROPERTY()
	TSet<TObjectPtr<UObject>> PendingOwners;
	bool bDraining = false;
public:

private:
	struct FPresentation
	{
		TWeakObjectPtr<UNiagaraComponent> Component;
		bool bImposedPause = false;
	};
	TArray<FPresentation> Presentation;
	TSet<TWeakObjectPtr<UNiagaraComponent>> Registered;
	bool bWasPaused = false;
	void OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaSeconds);
	void ApplyPause(FPresentation& Entry, bool bPaused);
	UFUNCTION()
	void OnNiagaraFinished(UNiagaraComponent* Component);
};
