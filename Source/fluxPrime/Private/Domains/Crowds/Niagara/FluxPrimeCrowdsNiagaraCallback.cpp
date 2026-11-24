#include "Domains/Crowds/Niagara/FluxPrimeCrowdsNiagaraCallback.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileDamageHandler.h"
#include "fluxPrime.h"

void UFluxPrimeCrowdsNiagaraCallback::InitializedNiagaraCallback(FFluxPrimeCrowdsNiagaraCallbackContext Context)
{
	if (PauseDamageHandler) PauseDamageHandler->Shutdown();
	World = Context.world;
	Damage = Context.damage;
	IsDebug = Context.isDebug;
	if (!IsValid(World))
	{
		UE_LOG(LogFluxPrime, Error, TEXT("Niagara callback initialized without a valid world."));
		return;
	}
	PauseDamageHandler = NewObject<UFluxPrimeCrowdsProjectileDamageHandler>(this);
	PauseDamageHandler->Initialize(World, IsDebug);
	// Legacy components have the same single-particle, at-most-one-export contract.
	PauseDamageHandler->ReserveShot(0, 60.0f, 1);
}

void UFluxPrimeCrowdsNiagaraCallback::ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Data,
	UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset)
{
	if (!PauseDamageHandler || Data.IsEmpty()) return;
	FFluxPrimeProjectileParticleData Particle;
	Particle.Position = Data[0].Position;
	Particle.Velocity = Data[0].Velocity;
	Particle.Size = Data[0].Size;
	Particle.Damage = Damage;
	Particle.ShotId = 0;
	PauseDamageHandler->ReceiveProjectileParticles_Implementation({Particle}, NiagaraSystem, SimulationPositionOffset);
}
