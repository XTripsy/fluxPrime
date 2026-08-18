// Fill out your copyright notice in the Description page of Project Settings.


#include "FluxPrimeWorldSubsystem.h"

#include "EngineUtils.h"
#include "Cores/FluxPrimePayload.h"
#include "Crowds/FluxPrimeController.h"
#include "Crowds/Components/FluxPrimeCrowdsManager.h"

void UFluxPrimeWorldSubsystem::HandleWorldInitializedActors(const UWorld::FActorsInitializedParams& Params)
{
	if (Params.World != GetWorld()) return;

	CrowdsController = GetCrowdsController();
	if (!CrowdsController.IsValid()) UE_LOG(LogTemp, Error, TEXT("NULLL"));
}

AFluxPrimeController* UFluxPrimeWorldSubsystem::GetCrowdsController() const
{
	if (CrowdsController.IsValid()) return CrowdsController.Get();
	
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	for (TActorIterator<AFluxPrimeController> It(World); It; ++It) return *It;

	return nullptr;
}

void UFluxPrimeWorldSubsystem::ExecuteSpawnAction(const FInstancedStruct& payload)
{
	const FFluxPrimeSpawnPayload* pair = payload.GetPtr<FFluxPrimeSpawnPayload>();
	if (!pair) return;
	
	UFluxPrimeCrowdsManager* manager = CrowdsController.Get()->GetCrowdsManager();
	manager->SpawnCrowd(pair->Identity, pair->Location, pair->Rotation);
}

void UFluxPrimeWorldSubsystem::ExecuteStateAction(const FInstancedStruct& payload)
{
	const FFluxPrimeStatePayload* pair = payload.GetPtr<FFluxPrimeStatePayload>();
	if (!pair) return;
	
	UFluxPrimeCrowdsManager* manager = CrowdsController.Get()->GetCrowdsManager();
	manager->ChangeState(pair->NewState);
}

void UFluxPrimeWorldSubsystem::ExecuteDamageAction(const FInstancedStruct& payload)
{
	const FFluxPrimeDamagePayload* pair = payload.GetPtr<FFluxPrimeDamagePayload>();
	if (!pair) return;
	
	UFluxPrimeCrowdsManager* manager = CrowdsController.Get()->GetCrowdsManager();
	manager->TakeDamage(pair->InstanceID, pair->ISMC);
}

void UFluxPrimeWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FWorldDelegates::OnWorldInitializedActors.AddUObject(
		this,
		&UFluxPrimeWorldSubsystem::HandleWorldInitializedActors
	);
}

void UFluxPrimeWorldSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldInitializedActors.RemoveAll(this);
	
	Super::Deinitialize();
}

void UFluxPrimeWorldSubsystem::RequestAction(const FInstancedStruct& payload)
{
	check(CrowdsController.IsValid());
	
	if (payload.GetScriptStruct() == FFluxPrimeSpawnPayload::StaticStruct()) ExecuteSpawnAction(payload);
	if (payload.GetScriptStruct() == FFluxPrimeStatePayload::StaticStruct()) ExecuteStateAction(payload);
	if (payload.GetScriptStruct() == FFluxPrimeDamagePayload::StaticStruct()) ExecuteDamageAction(payload);
}
