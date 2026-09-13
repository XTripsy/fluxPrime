#pragma once

#include "CoreMinimal.h"
#include "Generics/Systems/Ability/FluxPrimeBaseAbility.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsNiagaraCallback.h"
#include "FluxPrimeCrowdsProjectileAbility.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsProjectileAbilitySystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsNiagaraCallback> niagaraCallback = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsProjectileAbility : public FFluxPrimeBaseAbility
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsNiagaraCallback> NiagaraCallback = nullptr;
	
public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> NiagaraSystem = nullptr;
	
public:
	void InitializedProjectileAbilitySystems(FFluxPrimeCrowdsProjectileAbilitySystemsContext context)
	{
		check(context.world);
		check(context.niagaraCallback);
		check(NiagaraSystem);
		
		World = context.world;
		NiagaraCallback = context.niagaraCallback;
	}
	
	void Execute(FVector location, FRotator rotation, float damage)
	{
		UE_LOG(LogTemp, Log, TEXT("ABILITY:: PROJECTILE ATTACK"));
		
		UNiagaraComponent* projectileFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			NiagaraSystem,
			location + FVector::UpVector * 150,
			rotation
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