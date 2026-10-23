// Fill out your copyright notice in the Description page of Project Settings.


#include "Domains/Crowds/FluxPrimeCrowdsWorldSubsystem.h"

#include "Domains/Crowds/FluxPrimeCrowdsController.h"
#include "Domains/Crowds/Managers/FluxPrimeCrowdsManager.h"

AFluxPrimeCrowdsController* UFluxPrimeCrowdsWorldSubsystem::GetCrowdsController() const
{
	if (!GetBaseController()) return nullptr;
	return Cast<AFluxPrimeCrowdsController>(GetBaseController());
}

void UFluxPrimeCrowdsWorldSubsystem::HandleWorldInitializedActors(const UWorld::FActorsInitializedParams& Params)
{
	Super::HandleWorldInitializedActors(Params);
	
	CrowdsController = Cast<AFluxPrimeCrowdsController>(GetBaseController());
}

void UFluxPrimeCrowdsWorldSubsystem::RequestAction(const FInstancedStruct& payload)
{
	if (!CrowdsController.IsValid())
	{
		UE_LOG(LogTemp, Log, TEXT("CROWDS SUBSYSTEM:: CROWDS CONTROLLER NULL"));
		return;
	}
	
	UFluxPrimeCrowdsManager* manager = CrowdsController.Get()->GetCrowdsManager();
	if (!manager) return;
	
	manager->RequestAction(payload);
}

TMap<FUniqueNetIdRepl, int32> UFluxPrimeCrowdsWorldSubsystem::GetPlayerKillsData() const
{
	UFluxPrimeCrowdsManager* manager = CrowdsController.Get()->GetCrowdsManager();
	return manager->GetPlayerKillsData(); 
}
