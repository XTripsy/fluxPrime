#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Generics/Systems/Ability/FluxPrimeBaseAbility.h"
#include "Kismet/GameplayStatics.h"
#include "FluxPrimeCrowdsMeleAbility.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsMeleAbilitySystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsMeleAbility : public FFluxPrimeBaseAbility
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
public:
	UPROPERTY(EditAnywhere, Category = "Melee", meta = (ClampMin = "0", Units = "cm"))
	float TraceForwardDistance = 200.0f;
	UPROPERTY(EditAnywhere, Category = "Melee", meta = (ClampMin = "0", Units = "cm"))
	float TraceSphereRadius = 50.0f;
	UPROPERTY(EditAnywhere, Category = "Melee", meta = (Units = "cm"))
	float TraceHeightOffset = 75.0f;
	UPROPERTY(EditAnywhere, Category = "Melee")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	void InitializedMeleAbilitySystems(FFluxPrimeCrowdsMeleAbilitySystemsContext context)
	{
		check(context.world);
		
		World = context.world;
	}
	
	void Execute(FVector location, FRotator rotation, float damage)
	{
		FCollisionQueryParams params(SCENE_QUERY_STAT(AbilityMeleTrace), false);

		FHitResult hit;
		const FVector forward = rotation.Vector();

		const FVector start = location + FVector::UpVector * TraceHeightOffset;
		const FVector end   = start + forward * TraceForwardDistance;

		const bool bHit = World->SweepSingleByChannel(
			hit,
			start,
			end,
			FQuat::Identity,
			TraceChannel,
			FCollisionShape::MakeSphere(TraceSphereRadius),
			params
		);

		if (!bHit) return;

		UGameplayStatics::ApplyDamage(
				hit.GetActor(),
				damage,
				nullptr,
				nullptr,
				nullptr);
		
		UE_LOG(LogFluxPrime, Verbose, TEXT("Melee ability dealt %f damage to %s"), damage,
			hit.GetActor() ? *hit.GetActor()->GetName() : TEXT("null actor"));
	}
};