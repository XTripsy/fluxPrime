#pragma once

#include "CoreMinimal.h"
#include "Generics/Systems/Ability/FluxPrimeBaseAbility.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsNiagaraCallback.h"
#include "UObject/Package.h"
#include "FluxPrimeCrowdsProjectileAbility.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsProjectileAbilitySystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsNiagaraCallback> niagaraCallback = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsProjectileAbility : public FFluxPrimeBaseAbility
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsNiagaraCallback> NiagaraCallback = nullptr;

	/** GC backstop for per-shot callbacks; Niagara also references them through User.NiagaraCallback. */
	UPROPERTY()
	TArray<TObjectPtr<UFluxPrimeCrowdsNiagaraCallback>> ActiveNiagaraCallbacks;
	
	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (Units = "cm"))
	float TraceHeightOffset = 75.0f;
	
	UPROPERTY(EditAnywhere, Category = "Projectile")
	bool IsDebug = false;
	
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;
	
public:
	void InitializedProjectileAbilitySystems(FFluxPrimeCrowdsProjectileAbilitySystemsContext context)
	{
		check(context.world);
		check(context.niagaraCallback);
		check(NiagaraSystem);
		
		World = context.world;
		NiagaraCallback = context.niagaraCallback;
	}
	
	void Execute(FVector location, FRotator rotation, float damage)
	{
		UNiagaraComponent* projectileFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			NiagaraSystem,
			location + FVector::UpVector * TraceHeightOffset,
			rotation
		);

		if (!IsValid(projectileFX))
		{
			UE_LOG(LogFluxPrime, Warning, TEXT("Projectile ability %s could not spawn its Niagara system in world %s."),
				*Identity.ToString(), World ? *World->GetName() : TEXT("null"));
			return;
		}

		// One callback per shot so concurrent effects cannot overwrite each other's damage.
		UFluxPrimeCrowdsNiagaraCallback* particleCallback = NewObject<UFluxPrimeCrowdsNiagaraCallback>(GetTransientPackage());
		FFluxPrimeCrowdsNiagaraCallbackContext callbackContext;
		callbackContext.world = World;
		callbackContext.damage = damage;
		callbackContext.isDebug = IsDebug;
		particleCallback->InitializedNiagaraCallback(callbackContext);

		ActiveNiagaraCallbacks.Add(particleCallback);
		if (ActiveNiagaraCallbacks.Num() > 512)
		{
			ActiveNiagaraCallbacks.RemoveAt(0);
		}

		projectileFX->SetVariableFloat(
			TEXT("User.Damage"),
			damage
		);
		
		projectileFX->SetVariableObject(
			TEXT("User.NiagaraCallback"),
			particleCallback
		);
	}
};