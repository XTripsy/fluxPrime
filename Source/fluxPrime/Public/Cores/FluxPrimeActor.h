// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FluxPrimeActor.generated.h"

UCLASS()
class FLUXPRIME_API AFluxPrimeActor : public AActor
{
	GENERATED_BODY()

public:
	AFluxPrimeActor();

protected:
	virtual void BeginPlay() override;
};
