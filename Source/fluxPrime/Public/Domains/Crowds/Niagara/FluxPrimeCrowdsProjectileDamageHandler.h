#pragma once

#include "CoreMinimal.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileCallbackHandler.h"
#include "UObject/Object.h"
#include "FluxPrimeCrowdsProjectileDamageHandler.generated.h"

/**
 * Applies projectile damage with the same sphere sweep the per-shot callback used, but reads the
 * damage from each particle instead of from a per-shot object. One handler serves the whole batch.
 */
UCLASS()
class FLUXPRIME_API UFluxPrimeCrowdsProjectileDamageHandler : public UObject, public IFluxPrimeCrowdsProjectileCallbackHandler
{
	GENERATED_BODY()

public:
	void Initialize(UWorld* InWorld);
	/** Admission reserves one result slot; lifetime must exceed the asset's maximum particle lifetime. */
	bool ReserveShot(int32 ShotId, float MaxLifetimeSeconds, int32 Capacity);
	void CancelShot(int32 ShotId);
	void Shutdown();
	void FlushPendingDamage();

	virtual void ReceiveProjectileParticles_Implementation(const TArray<FFluxPrimeProjectileParticleData>& Particles,
		UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset) override;

private:
	UPROPERTY()
	TObjectPtr<UWorld> World;

	struct FAcceptedShot
	{
		double Expiry = 0.0;
		TWeakObjectPtr<AActor> Target;
		float Damage = 0.0f;
		bool bReceived = false;
	};
	TMap<int32, FAcceptedShot> AcceptedShots;
	bool bUsesReservations = false;
	bool bLoggedMissingWorld = false;
};
