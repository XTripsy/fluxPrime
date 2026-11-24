#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Generics/Systems/Ability/FluxPrimeBaseAbility.h"
#include "NiagaraFunctionLibrary.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileDiagnostics.h"
#include "Domains/Crowds/FluxPrimeCrowdsProjectileBatchSubsystem.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileTypes.h"
#include "Engine/World.h"
#include "Framework/Core/FluxPrimePauseWorldSubsystem.h"
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
	
	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (Units = "cm"))
	float TraceHeightOffset = 75.0f;
	
	UPROPERTY(EditAnywhere, Category = "Projectile")
	bool IsDebug = false;
	
public:
	/** Persistent system that renders every projectile at once. Required when bUseBatchedProjectiles is on. */
	UPROPERTY(EditAnywhere, Category = "Projectile|Batched")
	TObjectPtr<UNiagaraSystem> BatchedProjectileSystem = nullptr;

	/** Optional persistent CPU NDC muzzle system. Leave empty when muzzle emitters are in BatchedProjectileSystem. */
	UPROPERTY(EditAnywhere, Category = "Projectile|Batched")
	TObjectPtr<UNiagaraSystem> MuzzleFlashSystem = nullptr;

	/** Channel the batched system reads its spawn records from. Required when bUseBatchedProjectiles is on. */
	UPROPERTY(EditAnywhere, Category = "Projectile|Batched")
	TObjectPtr<UNiagaraDataChannelAsset> ProjectileSpawnChannel = nullptr;

	/**
	 * Routes shots through the shared projectile system instead of spawning one Niagara system per shot.
	 * Falls back to the full per-shot system if configuration is missing or submission is rejected.
	 */
	UPROPERTY(EditAnywhere, Category = "Projectile|Batched")
	bool bUseBatchedProjectiles = false;

	UPROPERTY(EditAnywhere, Category = "Projectile|Batched", meta = (ClampMin = "1"))
	int32 MaxInFlightShots = 16384;

	/** Set above the asset's maximum lifetime plus callback latency. */
	UPROPERTY(EditAnywhere, Category = "Projectile|Batched", meta = (ClampMin = "0.1"))
	float ReservationLifetimeSeconds = 60.0f;
	
public:
	void InitializedProjectileAbilitySystems(FFluxPrimeCrowdsProjectileAbilitySystemsContext context)
	{
		check(context.world);
		
		World = context.world;
	}
	
	void Execute(FVector location, FRotator rotation, float damage)
	{
		if (UFluxPrimePauseWorldSubsystem::IsWorldPaused(World)) return;
		if (!TrySubmitBatchedShot(location, rotation, damage)) UE_LOG(LogFluxPrime, Verbose, TEXT("PROJECTILE ABILITY :: FAIL TRY BATCHED SHOT"));
	}

private:
	/**
	 * Publishes the shot into the shared system. Returns false when batching is off or not configured,
	 * which leaves the caller on the unchanged per-shot path.
	 */
	bool TrySubmitBatchedShot(const FVector& location, const FRotator& rotation, float damage)
	{
		if (!bUseBatchedProjectiles || !IsValid(World))
		{
			FFluxPrimeCrowdsProjectileDiagnostics::RecordResult(!bUseBatchedProjectiles
				? EFluxPrimeProjectileRejection::Disabled : EFluxPrimeProjectileRejection::InvalidWorld);
			return false;
		}

		UFluxPrimeCrowdsProjectileBatchSubsystem* Batch = World->GetSubsystem<UFluxPrimeCrowdsProjectileBatchSubsystem>();
		if (Batch == nullptr)
		{
			FFluxPrimeCrowdsProjectileDiagnostics::RecordResult(EFluxPrimeProjectileRejection::MissingSubsystem);
			return false;
		}

		FFluxPrimeProjectileBatchConfig Config;
		Config.System = BatchedProjectileSystem;
		Config.MuzzleSystem = MuzzleFlashSystem;
		Config.SpawnChannel = ProjectileSpawnChannel;
		Config.MaxInFlightShots = MaxInFlightShots;
		Config.ReservationLifetimeSeconds = ReservationLifetimeSeconds;

		FFluxPrimeProjectileShot Shot;
		Shot.Location = location;
		Shot.Rotation = rotation;
		Shot.Damage = damage;
		Shot.HeightOffset = TraceHeightOffset;
		Shot.bDebug = IsDebug;

		if (!Batch->SubmitShot(Config, Shot))
		{
			// Terminal rejection must not bypass pause/admission through the legacy fallback.
			return Batch->GetLastRejection() == EFluxPrimeProjectileRejection::Paused
				|| Batch->GetLastRejection() == EFluxPrimeProjectileRejection::CapacityExceeded;
		}

		return true;
	}
};