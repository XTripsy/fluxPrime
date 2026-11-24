#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileDamageHandler.h"

#include "DrawDebugHelpers.h"
#include "Framework/Core/FluxPrimePauseWorldSubsystem.h"
#include "fluxPrime.h"
#include "Kismet/GameplayStatics.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileDiagnostics.h"

bool UFluxPrimeCrowdsProjectileDamageHandler::ReserveShot(int32 ShotId, float MaxLifetimeSeconds, int32 Capacity)
{
	check(IsInGameThread());
	bUsesReservations = true;
	if (!IsValid(World) || World->IsPaused() || Capacity <= 0 || !FMath::IsFinite(MaxLifetimeSeconds) || MaxLifetimeSeconds <= 0) return false;
	// Reclaim only when capacity is needed, not once per live projectile every frame.
	if (AcceptedShots.Num() >= Capacity)
	{
		const double Now = World->GetTimeSeconds();
		for (auto It = AcceptedShots.CreateIterator(); It; ++It)
		{
			if (!It.Value().bReceived && It.Value().Expiry < Now) It.RemoveCurrent();
		}
	}
	if (AcceptedShots.Num() >= Capacity || AcceptedShots.Contains(ShotId)) return false;
	FAcceptedShot Shot;
	Shot.Expiry = World->GetTimeSeconds() + MaxLifetimeSeconds;
	AcceptedShots.Add(ShotId, Shot);
	return true;
}

void UFluxPrimeCrowdsProjectileDamageHandler::CancelShot(int32 ShotId) { AcceptedShots.Remove(ShotId); }

void UFluxPrimeCrowdsProjectileDamageHandler::Shutdown()
{
	if (IsValid(World))
	{
		if (auto* Pause = World->GetSubsystem<UFluxPrimePauseWorldSubsystem>())
		{
			Pause->OnResume.RemoveAll(this);
			Pause->ReleasePendingOwner(this);
		}
	}
	AcceptedShots.Reset();
	World = nullptr;
}

void UFluxPrimeCrowdsProjectileDamageHandler::FlushPendingDamage()
{
	if (!IsValid(World) || World->IsPaused()) return;
	TArray<int32> Ready;
	for (auto It = AcceptedShots.CreateIterator(); It; ++It)
	{
		if (It.Value().bReceived)
		{
			Ready.Add(It.Key());
		}
	}
	for (int32 Id : Ready)
	{
		if (!IsValid(World) || World->IsPaused()) return;
		FAcceptedShot Shot;
		if (!AcceptedShots.RemoveAndCopyValue(Id, Shot)) continue;
		if (AActor* Target = Shot.Target.Get())
		{
			FFluxPrimeCrowdsProjectileDiagnostics::RecordDamage();
			UGameplayStatics::ApplyDamage(Target, Shot.Damage, nullptr, nullptr, nullptr);
		}
	}
	if (IsValid(World))
	{
		if (auto* Pause = World->GetSubsystem<UFluxPrimePauseWorldSubsystem>()) Pause->ReleasePendingOwner(this);
	}
}

void UFluxPrimeCrowdsProjectileDamageHandler::Initialize(UWorld* InWorld, bool bInDebug)
{
	if (!IsValid(InWorld))
	{
		UE_LOG(LogFluxPrime, Error, TEXT("Projectile damage handler %s initialized without a valid world; projectile damage is disabled."), *GetName());
		World = nullptr;
		return;
	}

	if (World != InWorld)
	{
		Shutdown();
		World = InWorld;
		if (auto* Pause = World->GetSubsystem<UFluxPrimePauseWorldSubsystem>())
			Pause->OnResume.AddUObject(this, &ThisClass::FlushPendingDamage);
	}
	bDebug = bInDebug;
	bLoggedMissingWorld = false;
}

void UFluxPrimeCrowdsProjectileDamageHandler::ReceiveProjectileParticles_Implementation(
	const TArray<FFluxPrimeProjectileParticleData>& Particles, UNiagaraSystem* NiagaraSystem,
	const FVector& SimulationPositionOffset)
{
	FFluxPrimeCrowdsProjectileDiagnostics::RecordSamples(Particles.Num());
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
		if (!IsValid(World)) return;
		FAcceptedShot* Accepted = bUsesReservations ? AcceptedShots.Find(Particle.ShotId) : nullptr;
		if (bUsesReservations && (!Accepted || Accepted->bReceived)) continue;
		if (Accepted) Accepted->bReceived = true;
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

		if (!bHit)
		{
			if (Accepted) AcceptedShots.Remove(Particle.ShotId);
			continue;
		}
		if (Accepted)
		{
			Accepted->Target = Hit.GetActor();
			Accepted->Damage = Particle.Damage;
			if (World->IsPaused())
			{
				if (auto* Pause = World->GetSubsystem<UFluxPrimePauseWorldSubsystem>()) Pause->RetainPendingOwner(this);
				continue;
			}
			AcceptedShots.Remove(Particle.ShotId);
		}
		else if (World->IsPaused()) continue;

		if (bDebug)
		{
			DrawDebugSphere(World, Start, 50.0f, 16, FColor::Yellow, false, 1.0f);
		}

		FFluxPrimeCrowdsProjectileDiagnostics::RecordDamage();
		UGameplayStatics::ApplyDamage(Hit.GetActor(), Particle.Damage, nullptr, nullptr, nullptr);

		UE_LOG(LogFluxPrime, VeryVerbose, TEXT("Projectile particle %d hit %s for %f damage"),
			Particle.ShotId, Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("null actor"), Particle.Damage);
	}
}
