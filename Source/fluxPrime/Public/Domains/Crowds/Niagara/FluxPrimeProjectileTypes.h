#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeProjectileTypes.generated.h"

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

	UPROPERTY(BlueprintReadWrite, Category = "Projectile")
	bool bDebug = false;
};

/** Everything the batch subsystem needs to own a persistent projectile system. */
USTRUCT(BlueprintType)
struct FFluxPrimeProjectileBatchConfig
{
	GENERATED_BODY()

	/** The single persistent system that renders every projectile. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TObjectPtr<UNiagaraSystem> System = nullptr;

	/** Channel the persistent system reads spawn records from. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TObjectPtr<UNiagaraDataChannelAsset> SpawnChannel = nullptr;
};
