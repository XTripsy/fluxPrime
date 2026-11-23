#include "Domains/Crowds/Niagara/FluxPrimeProjectileDamageHandler.h"

#include "DrawDebugHelpers.h"
#include "fluxPrime.h"
#include "Kismet/GameplayStatics.h"

void UFluxPrimeProjectileDamageHandler::Initialize(UWorld* InWorld, bool bInDebug)
{
	if (!IsValid(InWorld))
	{
		UE_LOG(LogFluxPrime, Error, TEXT("Projectile damage handler %s initialized without a valid world; projectile damage is disabled."), *GetName());
		World = nullptr;
		return;
	}

	World = InWorld;
	bDebug = bInDebug;
	bLoggedMissingWorld = false;
}

void UFluxPrimeProjectileDamageHandler::ReceiveProjectileParticles_Implementation(
	const TArray<FFluxPrimeProjectileParticleData>& Particles, UNiagaraSystem* NiagaraSystem,
	const FVector& SimulationPositionOffset)
{
	if (!IsValid(World))
	{
		if (!bLoggedMissingWorld)
		{
			UE_LOG(LogFluxPrime, Error, TEXT("Projectile damage handler %s received particles but has no valid world; ignoring them."), *GetName());
			bLoggedMissingWorld = true;
		}
		return;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AbilityMeleTrace), false);
	FHitResult Hit;

	for (const FFluxPrimeProjectileParticleData& Particle : Particles)
	{
		const FVector Start = Particle.Position;

		const bool bHit = World->SweepSingleByChannel(
			Hit,
			Start,
			Start,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(50.0f),
			Params
		);

		if (!bHit) continue;

		if (bDebug)
		{
			DrawDebugSphere(World, Start, 50.0f, 16, FColor::Yellow, false, 1.0f);
		}

		UGameplayStatics::ApplyDamage(Hit.GetActor(), Particle.Damage, nullptr, nullptr, nullptr);

		UE_LOG(LogFluxPrime, VeryVerbose, TEXT("Projectile particle %d hit %s for %f damage"),
			Particle.ShotId, Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("null actor"), Particle.Damage);
	}
}
