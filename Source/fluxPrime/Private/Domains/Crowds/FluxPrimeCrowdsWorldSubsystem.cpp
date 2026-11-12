// Fill out your copyright notice in the Description page of Project Settings.


#include "Domains/Crowds/FluxPrimeCrowdsWorldSubsystem.h"
#include "fluxPrime.h"

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
		const UWorld* World = GetWorld();
		UE_LOG(LogFluxPrime, Warning, TEXT("Cannot dispatch action: crowds controller is not available in world %s"),
			World ? *World->GetName() : TEXT("unknown"));
		return;
	}
	
	UFluxPrimeCrowdsManager* manager = CrowdsController.Get()->GetCrowdsManager();
	if (!manager) return;
	
	manager->RequestAction(payload);
}

TMap<FUniqueNetIdRepl, int32> UFluxPrimeCrowdsWorldSubsystem::GetPlayerKillsData() const
{
	AFluxPrimeCrowdsController* CrowdsControllerPtr = GetCrowdsController();
	if (!CrowdsControllerPtr)
	{
		UE_LOG(LogFluxPrime, Warning, TEXT("Cannot read player kills: crowds controller is not available."));
		return {};
	}

	UFluxPrimeCrowdsManager* Manager = CrowdsControllerPtr->GetCrowdsManager();
	if (!Manager)
	{
		UE_LOG(LogFluxPrime, Warning, TEXT("Cannot read player kills: crowds manager is not initialized on controller %s."),
			*CrowdsControllerPtr->GetName());
		return {};
	}

	return Manager->GetPlayerKillsData();
}
