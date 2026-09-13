// Fill out your copyright notice in the Description page of Project Settings.


#include "Domains/Crowds/FluxPrimeCrowdsWorldSubsystem.h"

#include "Domains/Crowds/Data/FluxPrimeCrowdsPayloadData.h"
#include "Domains/Crowds/FluxPrimeCrowdsController.h"
#include "Domains/Crowds/Managers/FluxPrimeCrowdsManager.h"

AFluxPrimeCrowdsController* UFluxPrimeCrowdsWorldSubsystem::GetCrowdsController() const
{
	if (!GetBaseController()) return nullptr;
	return Cast<AFluxPrimeCrowdsController>(GetBaseController());
}

void UFluxPrimeCrowdsWorldSubsystem::ExecuteSpawnAction(const FInstancedStruct& payload)
{
	const FFluxPrimeSpawnPayload* pair = payload.GetPtr<FFluxPrimeSpawnPayload>();
	if (!pair) return;
	
	UFluxPrimeCrowdsManager* manager = CrowdsController.Get()->GetCrowdsManager();
	manager->SpawnCrowd(pair->Identity, pair->Location, pair->Rotation, pair->NewState);
}

void UFluxPrimeCrowdsWorldSubsystem::ExecuteDamageAction(const FInstancedStruct& payload)
{
	const FFluxPrimeDamagePayload* pair = payload.GetPtr<FFluxPrimeDamagePayload>();
	if (!pair) return;
	
	UFluxPrimeCrowdsManager* manager = CrowdsController.Get()->GetCrowdsManager();
	manager->TakeDamage(pair->InstanceID, pair->ISMC, pair->CrowdDamageTaken);
}

void UFluxPrimeCrowdsWorldSubsystem::HandleWorldInitializedActors(const UWorld::FActorsInitializedParams& Params)
{
	Super::HandleWorldInitializedActors(Params);
	
	CrowdsController = Cast<AFluxPrimeCrowdsController>(GetBaseController());
}

void UFluxPrimeCrowdsWorldSubsystem::RequestAction(const FInstancedStruct& payload)
{
	check(CrowdsController.IsValid());
	
	if (payload.GetScriptStruct() == FFluxPrimeSpawnPayload::StaticStruct()) ExecuteSpawnAction(payload);
	if (payload.GetScriptStruct() == FFluxPrimeDamagePayload::StaticStruct()) ExecuteDamageAction(payload);
}
