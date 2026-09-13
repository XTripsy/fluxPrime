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
};

UCLASS()
class FLUXPRIME_API UFluxPrimeCrowdsNiagaraCallback : public UObject, public INiagaraParticleCallbackHandler
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
public:
	void InitializedNiagaraCallback(FFluxPrimeCrowdsNiagaraCallbackContext context);
	virtual void ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Data, UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset) override;
};