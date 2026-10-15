// Fill out your copyright notice in the Description page of Project Settings.


#include "Domains/Crowds/Niagara/FluxPrimeCrowdsNiagaraCallback.h"

#include "Kismet/GameplayStatics.h"

void UFluxPrimeCrowdsNiagaraCallback::InitializedNiagaraCallback(FFluxPrimeCrowdsNiagaraCallbackContext context)
{
	check(context.world);
	
	World = context.world;
}

void UFluxPrimeCrowdsNiagaraCallback::ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Data,
                                                                   UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset)
{
	UE_LOG(LogTemp, Log, TEXT("NIAGARACALLBACK:: HIT"));
	
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AbilityMeleTrace), false);

	FHitResult Hit;

	FVector start = FVector::Zero();
	FVector end = FVector::Zero();
	
	for (const FBasicParticleData& particle : Data)
	{
		UE_LOG(
			LogTemp,
			Log,
			TEXT("HIT POSITION: %s"),
			*particle.Position.ToString()
		);
		
		UE_LOG(
			LogTemp,
			Log,
			TEXT("HIT DAMAGE: %f"),
			particle.Size
		);
		
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
		
		UGameplayStatics::ApplyDamage(
				Hit.GetActor(),
				particle.Size,
				nullptr,
				nullptr,
				nullptr);
		
		UE_LOG(LogTemp, Log, TEXT("ABILITY:: END PROJECTILE ATTACK"));
	}
}
