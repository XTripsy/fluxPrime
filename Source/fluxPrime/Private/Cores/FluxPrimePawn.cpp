// Fill out your copyright notice in the Description page of Project Settings.


#include "Cores/FluxPrimePawn.h"


AFluxPrimePawn::AFluxPrimePawn()
{
	
}

void AFluxPrimePawn::PostInitProperties()
{
	Super::PostInitProperties();
	PrimaryActorTick.bCanEverTick = IsUseTick;
}

void AFluxPrimePawn::BeginPlay()
{
	Super::BeginPlay();
}

void AFluxPrimePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}