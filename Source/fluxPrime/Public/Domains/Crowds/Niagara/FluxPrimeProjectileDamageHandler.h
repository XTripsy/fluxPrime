#pragma once

#include "CoreMinimal.h"
#include "Domains/Crowds/Niagara/FluxPrimeProjectileCallbackHandler.h"
#include "UObject/Object.h"
#include "FluxPrimeProjectileDamageHandler.generated.h"

/**
 * Applies projectile damage with the same sphere sweep the per-shot callback used, but reads the
 * damage from each particle instead of from a per-shot object. One handler serves the whole batch.
 */
UCLASS()
class FLUXPRIME_API UFluxPrimeProjectileDamageHandler : public UObject, public IFluxPrimeProjectileCallbackHandler
{
	GENERATED_BODY()

public:
	void Initialize(UWorld* InWorld, bool bInDebug);

	virtual void ReceiveProjectileParticles_Implementation(const TArray<FFluxPrimeProjectileParticleData>& Particles,
		UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset) override;

private:
	UPROPERTY()
	TObjectPtr<UWorld> World;

	bool bDebug = false;
	bool bLoggedMissingWorld = false;
};
