// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NiagaraDataInterfaceExport.h"
#include "FluxPrimeNiagaraCallback.generated.h"

USTRUCT()
struct FFluxPrimeNiagaraCallbackContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
};

UCLASS()
class FLUXPRIME_API UFluxPrimeNiagaraCallback : public UObject, public INiagaraParticleCallbackHandler
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
public:
	void InitializedNiagaraCallback(FFluxPrimeNiagaraCallbackContext context);
	virtual void ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Data, UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset) override;
};