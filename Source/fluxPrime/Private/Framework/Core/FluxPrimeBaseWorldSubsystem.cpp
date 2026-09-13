// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Core/FluxPrimeBaseWorldSubsystem.h"

#include "EngineUtils.h"
#include "Framework/Core/FluxPrimeBaseController.h"

AFluxPrimeBaseController* UFluxPrimeBaseWorldSubsystem::GetBaseController() const
{
	if (Controller.IsValid()) return Controller.Get();
	
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	for (TActorIterator<AFluxPrimeBaseController> It(World); It; ++It) return *It;

	return nullptr;
}

void UFluxPrimeBaseWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	FWorldDelegates::OnWorldInitializedActors.AddUObject(
		this,
		&UFluxPrimeBaseWorldSubsystem::HandleWorldInitializedActors
	);
}

void UFluxPrimeBaseWorldSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldInitializedActors.RemoveAll(this);
	
	Super::Deinitialize();
}

void UFluxPrimeBaseWorldSubsystem::HandleWorldInitializedActors(const UWorld::FActorsInitializedParams& Params)
{
	if (Params.World != GetWorld()) return;

	Controller = GetBaseController();
	if (!Controller.IsValid()) UE_LOG(LogTemp, Error, TEXT("NULLL"));
}
