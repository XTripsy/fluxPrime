#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseAbilitySystem.h"
#include "FluxPrimeMeleAbilitySystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeMeleAbilitySystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
};

USTRUCT(BlueprintType)
struct FFluxPrimeMeleAbilitySystem : public FFluxPrimeBaseAbilitySystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
public:
	void InitializedMeleAbilitySystems(FFluxPrimeMeleAbilitySystemsContext context)
	{
		check(context.world);
		
		World = context.world;
	}
	
	void Execute(FVector location, float rotation, float damage)
	{
		UE_LOG(LogTemp, Log, TEXT("ABILITY:: START MELE ATTACK"));
		
		FCollisionQueryParams Params(SCENE_QUERY_STAT(AbilityMeleTrace), false);

		FHitResult Hit;
		const FVector forward = FRotator(0.0f, rotation + 85.0f, 0.0f).Vector();

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