#pragma once

#include "CoreMinimal.h"
#include "Generics/Systems/Ability/FluxPrimeBaseAbility.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsNiagaraCallback.h"
#include "Domains/Crowds/Niagara/FluxPrimeProjectileBatchSubsystem.h"
#include "Domains/Crowds/Niagara/FluxPrimeProjectileTypes.h"
#include "Engine/World.h"
#include "UObject/Package.h"
#include "FluxPrimeCrowdsProjectileAbility.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsProjectileAbilitySystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsProjectileAbility : public FFluxPrimeBaseAbility
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
	/** GC backstop for per-shot callbacks; Niagara also references them through User.NiagaraCallback. */
	UPROPERTY()
	TArray<TObjectPtr<UFluxPrimeCrowdsNiagaraCallback>> ActiveNiagaraCallbacks;
	
	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (Units = "cm"))
	float TraceHeightOffset = 75.0f;
	
	UPROPERTY(EditAnywhere, Category = "Projectile")
	bool IsDebug = false;
	
public:
	/** Per-shot system used when batching is off, or as the fallback when a shot cannot be batched. */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;

	/** Persistent system that renders every projectile at once. Required when bUseBatchedProjectiles is on. */
	UPROPERTY(EditAnywhere, Category = "Projectile|Batched")
	TObjectPtr<UNiagaraSystem> BatchedProjectileSystem = nullptr;

	/** Channel the batched system reads its spawn records from. Required when bUseBatchedProjectiles is on. */
	UPROPERTY(EditAnywhere, Category = "Projectile|Batched")
	TObjectPtr<UNiagaraDataChannelAsset> ProjectileSpawnChannel = nullptr;

	/** Muzzle-only effect spawned per shot while batching. Leave empty to spawn no per-shot effect. */
	UPROPERTY(EditAnywhere, Category = "Projectile|Batched")
	TObjectPtr<UNiagaraSystem> MuzzleFlashSystem = nullptr;

	/**
	 * Routes shots through the shared projectile system instead of spawning one Niagara system per shot.
	 * Falls back to the full per-shot system if configuration is missing or submission is rejected.
	 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Batched")
	bool bUseBatchedProjectiles = false;
	
public:
	void InitializedProjectileAbilitySystems(FFluxPrimeCrowdsProjectileAbilitySystemsContext context)
	{
		check(context.world);
		
		World = context.world;
	}
	
	void Execute(FVector location, FRotator rotation, float damage)
	{
		if (TrySubmitBatchedShot(location, rotation, damage))
		{
			return;
		}

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

private:
	/**
	 * Publishes the shot into the shared system. Returns false when batching is off or not configured,
	 * which leaves the caller on the unchanged per-shot path.
	 */
	bool TrySubmitBatchedShot(const FVector& location, const FRotator& rotation, float damage)
	{
		if (!bUseBatchedProjectiles || !IsValid(BatchedProjectileSystem) || !IsValid(ProjectileSpawnChannel) || !IsValid(World))
		{
			return false;
		}

		UFluxPrimeProjectileBatchSubsystem* Batch = World->GetSubsystem<UFluxPrimeProjectileBatchSubsystem>();
		if (Batch == nullptr)
		{
			return false;
		}

		FFluxPrimeProjectileBatchConfig Config;
		Config.System = BatchedProjectileSystem;
		Config.SpawnChannel = ProjectileSpawnChannel;

		FFluxPrimeProjectileShot Shot;
		Shot.Location = location;
		Shot.Rotation = rotation;
		Shot.Damage = damage;
		Shot.HeightOffset = TraceHeightOffset;
		Shot.bDebug = IsDebug;

		if (!Batch->SubmitShot(Config, Shot))
		{
			return false;
		}

		if (IsValid(MuzzleFlashSystem))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				MuzzleFlashSystem,
				location + FVector::UpVector * TraceHeightOffset,
				rotation
			);
		}

		return true;
	}
};