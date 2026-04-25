// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FluxPrimeCrowdsSpawner.generated.h"

UCLASS()
class FLUXPRIME_API AFluxPrimeCrowdsSpawner : public AActor
{
	GENERATED_BODY()

public:
	AFluxPrimeCrowdsSpawner();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
