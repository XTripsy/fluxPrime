// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/FluxPrimeController.h"

#include "Crowds/Components/FluxPrimeCrowdsManager.h"
#include "Crowds/Components/FluxPrimeCrowdsNetManager.h"

AFluxPrimeController::AFluxPrimeController()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetMinNetUpdateFrequency(20);
	
	USceneComponent* sceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = sceneRoot;
	
	CrowdsNetComponent = CreateDefaultSubobject<UFluxPrimeCrowdsNetManager>(TEXT("CrowdsNet"));
	
	CrowdsManager = CreateDefaultSubobject<UFluxPrimeCrowdsManager>(TEXT("CrowdsManager"));
}

void AFluxPrimeController::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetActorLocation(FVector::ZeroVector);
}

void AFluxPrimeController::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		CrowdsManager->OnCrowdsManagerActionChange.BindUObject(CrowdsNetComponent, &UFluxPrimeCrowdsNetManager::OnActionChange);
	}
	else
	{
		CrowdsNetComponent->OnCrowdsNetManagerActionChange.BindUObject(CrowdsManager, &UFluxPrimeCrowdsManager::OnActionChange);
	}
	
	FFluxPrimeCrowdsManagerContext context;
	context.CrowdsComponents = &CrowdsComponents;
	CrowdsManager->InitializeManager(context);
		
	CrowdsManager->PreLoading();
}
