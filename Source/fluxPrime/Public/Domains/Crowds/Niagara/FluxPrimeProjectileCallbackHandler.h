#pragma once

#include "CoreMinimal.h"
#include "Domains/Crowds/Niagara/FluxPrimeProjectileTypes.h"
#include "UObject/Interface.h"
#include "FluxPrimeProjectileCallbackHandler.generated.h"

class UNiagaraSystem;

UINTERFACE(MinimalAPI, BlueprintType)
class UFluxPrimeProjectileCallbackHandler : public UInterface
{
	GENERATED_BODY()
};

/**
 * Receives projectile samples from the batched system. Mirrors INiagaraParticleCallbackHandler,
 * but carries the per-particle damage and shot id that FBasicParticleData cannot express.
 */
class FLUXPRIME_API IFluxPrimeProjectileCallbackHandler
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "FluxPrime|Projectile")
	void ReceiveProjectileParticles(const TArray<FFluxPrimeProjectileParticleData>& Particles,
		UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset);
};
