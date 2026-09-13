// Fill out your copyright notice in the Description page of Project Settings.


#include "Domains/Crowds/FluxPrimeCrowdsController.h"

#include "Domains/Crowds/Managers/FluxPrimeCrowdsManager.h"
#include "Framework/Net/FluxPrimeNetManager.h"

AFluxPrimeCrowdsController::AFluxPrimeCrowdsController()
{
	PrimaryActorTick.bCanEverTick = false;
	SetMinNetUpdateFrequency(20);
	
	CrowdsManager = CreateDefaultSubobject<UFluxPrimeCrowdsManager>(TEXT("CrowdsManager"));
}

void AFluxPrimeCrowdsController::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		CrowdsManager->OnCrowdsManagerActionChange.BindUObject(NetManager, &UFluxPrimeNetManager::OnActionChange);
	}
	else
	{
		NetManager->OnCrowdsNetManagerActionChange.BindUObject(CrowdsManager, &UFluxPrimeCrowdsManager::OnActionChange);
	}
	
	CrowdsManager->PreLoading();
}
