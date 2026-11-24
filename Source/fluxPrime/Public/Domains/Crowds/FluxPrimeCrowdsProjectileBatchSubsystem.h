#pragma once

#include "CoreMinimal.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileTypes.h"
#include "NiagaraDataChannelPublic.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileDiagnostics.h"
#include "Framework/Core/FluxPrimeBaseWorldSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "FluxPrimeCrowdsProjectileBatchSubsystem.generated.h"

class UFluxPrimeCrowdsProjectileDamageHandler;
class UNiagaraComponent;

/**
 * Owns one persistent Niagara component for every projectile in the world.
 *
 * Spawning a Niagara system per shot costs a per-instance concurrent tick, a per-instance post tick
 * and a game-thread wait in NiagaraPostActorTick for every live projectile. Submitting shots into a
 * data channel instead keeps a single system instance alive, so those costs stop scaling with the
 * number of projectiles in flight. Collision and damage stay inside Niagara and its callback.
 */
UCLASS()
class FLUXPRIME_API UFluxPrimeCrowdsProjectileBatchSubsystem : public UFluxPrimeBaseWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Turns a shot into the record the spawn data channel carries. */
	static FFluxPrimeProjectileSpawnRecord MakeSpawnRecord(const FVector& Location, const FRotator& Rotation,
		float Damage, float HeightOffset, int32 ShotId);

	/** Returns true only after publishing. On false, the caller must use the legacy path for this shot. */
	bool SubmitShot(const FFluxPrimeProjectileBatchConfig& Config, const FFluxPrimeProjectileShot& Shot);

	EFluxPrimeProjectileRejection GetLastRejection() const { return LastRejection; }

	void ShutdownBatch();

	UNiagaraComponent* GetBatchComponent() const { return BatchComponent; }

	virtual void Deinitialize() override;

private:
	bool EnsureBatchComponent(const FFluxPrimeProjectileBatchConfig& Config);
	bool PublishSpawnRecord(const FFluxPrimeProjectileBatchConfig& Config, const FFluxPrimeProjectileSpawnRecord& Record);

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> BatchComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> MuzzleComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsProjectileDamageHandler> DamageHandler = nullptr;

	UPROPERTY()
	TObjectPtr<UNiagaraDataChannelAsset> ActiveChannel = nullptr;

	EFluxPrimeProjectileRejection LastRejection = EFluxPrimeProjectileRejection::None;
	int32 NextShotId = 0;
};
