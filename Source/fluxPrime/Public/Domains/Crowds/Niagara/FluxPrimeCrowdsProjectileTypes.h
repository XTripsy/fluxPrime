#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeCrowdsProjectileTypes.generated.h"

class UNiagaraDataChannelAsset;
class UNiagaraSystem;

/** One projectile sample handed from Niagara to game code after the simulation tick. */
USTRUCT(BlueprintType)
struct FFluxPrimeProjectileParticleData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	FVector Position = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	float Size = 0.0f;

	/** Damage owned by the shot that produced this particle. */
	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	float Damage = 0.0f;

	/** Identifies the shot that produced this particle, so concurrent shots stay distinguishable. */
	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	int32 ShotId = INDEX_NONE;
};

/** A single shot published into the spawn data channel. */
USTRUCT(BlueprintType)
struct FFluxPrimeProjectileSpawnRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	FVector Position = FVector::ZeroVector;

	/** Normalized travel direction. The emitter keeps ownership of the speed. */
	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	FVector Direction = FVector::ForwardVector;

	/** Full world orientation, including roll; muzzle local vectors are rotated by this quaternion. */
	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	FQuat Orientation = FQuat::Identity;

	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	float Damage = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Projectile")
	int32 ShotId = INDEX_NONE;
};

/** A projectile fired by the ability, before it is turned into a spawn record. */
USTRUCT(BlueprintType)
struct FFluxPrimeProjectileShot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	float Damage = 0.0f;

	/** Lifts the shot off the muzzle to match the legacy spawn height. */
	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	float HeightOffset = 0.0f;
};

/** Everything the batch subsystem needs to own a persistent projectile system. */
USTRUCT(BlueprintType)
struct FFluxPrimeProjectileBatchConfig
{
	GENERATED_BODY()

	/** Maximum accepted shots awaiting collision or expiry, including deferred pause results. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (ClampMin = "1"))
	int32 MaxInFlightShots = 16384;

	/** Must exceed the Niagara maximum lifetime plus export delivery latency, in game seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (ClampMin = "0.1"))
	float ReservationLifetimeSeconds = 60.0f;

	/** The single persistent system that renders every projectile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TObjectPtr<UNiagaraSystem> System = nullptr;

	/** Optional separate persistent muzzle consumer of the same channel. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TObjectPtr<UNiagaraSystem> MuzzleSystem = nullptr;

	/** Channel the persistent system reads spawn records from. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TObjectPtr<UNiagaraDataChannelAsset> SpawnChannel = nullptr;
};
