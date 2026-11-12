// Fill out your copyright notice in the Description page of Project Settings.


#include "Domains/Crowds/Niagara/FluxPrimeCrowdsNiagaraCallback.h"
#include "fluxPrime.h"

#include "Kismet/GameplayStatics.h"

void UFluxPrimeCrowdsNiagaraCallback::InitializedNiagaraCallback(FFluxPrimeCrowdsNiagaraCallbackContext context)
{
	if (!IsValid(context.world))
	{
		UE_LOG(LogFluxPrime, Error, TEXT("Niagara callback %s initialized without a valid world; particle damage is disabled."), *GetName());
		World = nullptr;
		Damage = 0.0f;
		return;
	}

	World = context.world;
	Damage = context.damage;
	bLoggedMissingWorld = false;
}

void UFluxPrimeCrowdsNiagaraCallback::ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Data,
                                                                   UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset)
{
	if (!IsValid(World))
	{
		if (!bLoggedMissingWorld)
		{
			UE_LOG(LogFluxPrime, Error, TEXT("Niagara callback %s received particles but has no valid world; ignoring them."), *GetName());
			bLoggedMissingWorld = true;
		}
		return;
	}

	UE_LOG(LogFluxPrime, VeryVerbose, TEXT("Niagara callback received %d particles"), Data.Num());
	
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AbilityMeleTrace), false);

	FHitResult Hit;

	FVector start = FVector::Zero();
	FVector end = FVector::Zero();
	
	for (const FBasicParticleData& particle : Data)
	{
		start = particle.Position;
		end = start;
		
		const bool bHit = World->SweepSingleByChannel(
			Hit,
			start,
			end,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(50.0f),
			Params
		);

		if (!bHit) continue;

		DrawDebugSphere(
			World,
			end,
			50.0f,
			16,
			FColor::Yellow,
			false,
			1.0f
		);
		
		UGameplayStatics::ApplyDamage(
				Hit.GetActor(),
				Damage,
				nullptr,
				nullptr,
				nullptr);
		
		UE_LOG(LogFluxPrime, VeryVerbose, TEXT("Projectile particle hit %s for %f damage"),
			Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("null actor"), Damage);
	}
}
