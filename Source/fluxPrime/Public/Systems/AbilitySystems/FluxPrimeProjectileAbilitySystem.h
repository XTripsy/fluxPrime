#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseAbilitySystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Cores/FluxPrimeNiagaraCallback.h"
#include "FluxPrimeProjectileAbilitySystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeProjectileAbilitySystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeNiagaraCallback> niagaraCallback = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeProjectileAbilitySystem : public FFluxPrimeBaseAbilitySystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeNiagaraCallback> NiagaraCallback = nullptr;
	
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;
	
public:
	void InitializedProjectileAbilitySystems(FFluxPrimeProjectileAbilitySystemsContext context)
	{
		check(context.world);
		check(context.niagaraCallback);
		check(NiagaraSystem);
		
		World = context.world;
		NiagaraCallback = context.niagaraCallback;
	}
	
	void Execute(FVector location, float rotation, float damage)
	{
		UE_LOG(LogTemp, Log, TEXT("ABILITY:: PROJECTILE ATTACK"));
		
		UNiagaraComponent* projectileFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			NiagaraSystem,
			location + FVector::UpVector * 150,
			FRotator(0.0f, rotation + 85.0f, 0.0f)
		);
		
		projectileFX->SetVariableFloat(
			TEXT("User.Damage"),
			damage
		);
		
		projectileFX->SetVariableObject(
			TEXT("User.NiagaraCallback"),
			NiagaraCallback
		);
	}
};