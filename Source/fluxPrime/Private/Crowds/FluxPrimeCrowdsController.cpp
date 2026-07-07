// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/FluxPrimeCrowdsController.h"

#include "Crowds/Components/FluxPrimeCrowdsManager.h"
#include "Crowds/Components/FluxPrimeCrowdsNetComponent.h"

AFluxPrimeCrowdsController::AFluxPrimeCrowdsController()
{
	PrimaryActorTick.bCanEverTick = false;
	SetMinNetUpdateFrequency(20);
	
	USceneComponent* sceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = sceneRoot;
	
	CrowdsNetComponent = CreateDefaultSubobject<UFluxPrimeCrowdsNetComponent>(TEXT("CrowdsNet"));
	
	CrowdsManager = CreateDefaultSubobject<UFluxPrimeCrowdsManager>(TEXT("CrowdsManager"));
}

void AFluxPrimeCrowdsController::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetActorLocation(FVector::ZeroVector);
}

void AFluxPrimeCrowdsController::BeginPlay()
{
	Super::BeginPlay();
	
	SetReplicates(IsReplicated);
	
	{
		FFluxPrimeCrowdsManagerContext context;
		context.CrowdsComponents = &CrowdsComponents;
		context.CrowdsNetComponent = CrowdsNetComponent;
		context.isReplicated = IsReplicated;
		CrowdsManager->InitializeManager(context);
	}
	CrowdsManager->PreLoading();
}
