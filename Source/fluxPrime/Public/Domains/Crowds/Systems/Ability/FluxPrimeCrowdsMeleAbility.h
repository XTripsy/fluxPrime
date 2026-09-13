#pragma once

#include "CoreMinimal.h"
#include "Generics/Systems/Ability/FluxPrimeBaseAbility.h"
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
	void InitializedMeleAbilitySystems(FFluxPrimeCrowdsMeleAbilitySystemsContext context)
	{
		check(context.world);
		
		World = context.world;
	}
	
	void Execute(FVector location, FRotator rotation, float damage)
	{
		UE_LOG(LogTemp, Log, TEXT("ABILITY:: START MELE ATTACK"));
		
		FCollisionQueryParams Params(SCENE_QUERY_STAT(AbilityMeleTrace), false);

		FHitResult Hit;
		const FVector forward = rotation.Vector();

		const FVector start = location;
		const FVector end   = start + forward * 200.0f;

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
		
		UE_LOG(LogTemp, Log, TEXT("ABILITY:: END MELE ATTACK"));
	}
};