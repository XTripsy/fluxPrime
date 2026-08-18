// Fill out your copyright notice in the Description page of Project Settings.


#include "Cores/FluxPrimeNiagaraCallback.h"

void UFluxPrimeNiagaraCallback::InitializedNiagaraCallback(FFluxPrimeNiagaraCallbackContext context)
{
	check(context.world);
	
	World = context.world;
}

void UFluxPrimeNiagaraCallback::ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Data,
                                                                   UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset)
{
	UE_LOG(LogTemp, Log, TEXT("NIAGARACALLBACK:: HIT"));
	
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AbilityMeleTrace), false);

	FHitResult Hit;

	FVector start = FVector::Zero();
	FVector end = FVector::Zero();
	
	for (const FBasicParticleData& Particle : Data)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("HIT POSITION: %s"),
			*Particle.Position.ToString()
		);
		
		UE_LOG(
			LogTemp,
			Log,
			TEXT("HIT DAMAGE: %f"),
			Particle.Size
		);
		
		start = Particle.Position;
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

		if (!bHit) return;

		DrawDebugSphere(
			World,
			end,
			50.0f,
			16,
			FColor::Yellow,
			false,
			1.0f
		);
		
		UE_LOG(LogTemp, Log, TEXT("ABILITY:: END PROJECTILE ATTACK"));
	}
}
