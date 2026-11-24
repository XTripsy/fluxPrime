// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NiagaraDataInterfaceExport.h"
#include "FluxPrimeCrowdsNiagaraCallback.generated.h"

USTRUCT()
struct FFluxPrimeCrowdsNiagaraCallbackContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world;

	/** Damage applied to every particle hit. Set per effect instance rather than derived from particle size. */
	float damage = 0.0f;
	bool isDebug = false;
};

UCLASS()
class FLUXPRIME_API UFluxPrimeCrowdsNiagaraCallback : public UObject, public INiagaraParticleCallbackHandler
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World;

	UPROPERTY()
	TObjectPtr<class UFluxPrimeCrowdsProjectileDamageHandler> PauseDamageHandler;
	float Damage = 0.0f;
	bool bLoggedMissingWorld = false;
	bool IsDebug = false;
	
public:
	void InitializedNiagaraCallback(FFluxPrimeCrowdsNiagaraCallbackContext context);
	virtual void ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Data, UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset) override;
};