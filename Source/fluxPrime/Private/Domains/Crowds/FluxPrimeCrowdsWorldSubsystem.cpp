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
	const FFluxPrimeCrowdsSpawnPayload* pair = payload.GetPtr<FFluxPrimeCrowdsSpawnPayload>();
	if (!pair) return;
	
	UFluxPrimeCrowdsManager* manager = CrowdsController.Get()->GetCrowdsManager();
	manager->SpawnCrowd(pair->Identity, pair->Location, pair->Rotation, pair->RandomBoundingBox, pair->NewState, pair->TotalEntityToSpawn);
}

void UFluxPrimeCrowdsWorldSubsystem::ExecuteShootRayAction(const FInstancedStruct& payload)
{
	const FFluxPrimeCrowdsShootRayPayload* pair = payload.GetPtr<FFluxPrimeCrowdsShootRayPayload>();
	if (!pair) return;
	
	UFluxPrimeCrowdsManager* manager = CrowdsController.Get()->GetCrowdsManager();
	manager->ShootRay(pair->StartRayLocation, pair->EndRayLocation, pair->CrowdDamageTaken);
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
	if (payload.GetScriptStruct() == FFluxPrimeCrowdsSpawnPayload::StaticStruct()) ExecuteSpawnAction(payload);
	if (payload.GetScriptStruct() == FFluxPrimeCrowdsShootRayPayload::StaticStruct()) ExecuteShootRayAction(payload);
}
