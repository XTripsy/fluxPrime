#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FluxPrimeCrowdsProjectileTestTarget.generated.h"

/**
 * Damage sink for projectile automation tests. Records every damage event so a test can
 * assert how much damage arrived and how many separate applications produced it.
 */
UCLASS()
class AFluxPrimeCrowdsProjectileTestTarget : public AActor
{
	GENERATED_BODY()

public:
	AFluxPrimeCrowdsProjectileTestTarget();

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator,
		AActor* DamageCauser) override;

	TFunction<void()> OnTestDamage;
	float GetTotalDamage() const { return TotalDamage; }
	int32 GetDamageEventCount() const { return DamageEventCount; }

private:
	float TotalDamage = 0.0f;
	int32 DamageEventCount = 0;
};
