// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Core/FluxPrimeBaseController.h"

#include "Framework/Net/FluxPrimeNetManager.h"

AFluxPrimeBaseController::AFluxPrimeBaseController()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetMinNetUpdateFrequency(20);
		
	USceneComponent* sceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = sceneRoot;
		
	NetManager = CreateDefaultSubobject<UFluxPrimeNetManager>(TEXT("NetManager"));
}

void AFluxPrimeBaseController::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetActorLocation(FVector::ZeroVector);
}

void AFluxPrimeBaseController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	SetReplicates(IsReplicated);
}

TObjectPtr<UFluxPrimeNetManager> AFluxPrimeBaseController::GetNetManager()
{
	return (IsReplicated)? NetManager : nullptr;
}