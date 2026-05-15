// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeCrowdsSpawnerComponentInterface.h"
#include "Cores/FluxPrimeStruct.h"
#include "Components/ActorComponent.h"
#include "Systems/FluxPrimeNavigationSystems.h"
#include "FluxPrimeCrowdsSpawnerComponent.generated.h"

struct FInstancedStruct;

DECLARE_DELEGATE_OneParam(FOnSpawnCrowdsNet, const FInstancedStruct& payload);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLUXPRIME_API UFluxPrimeCrowdsSpawnerComponent : public UActorComponent, public IFluxPrimeCrowdsSpawnerComponentInterface
{
	GENERATED_BODY()

private:
	uint16* CrowdsActive;
	uint16* CrowdsTotal;
	FFluxPrimeCrowds* CrowdsData;
	FFluxPrimeNavigationSystems* NavigationSystems;
	
	UPROPERTY()
	TMap<FName, int8> CrowdsTypes;
	
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CrowdsComponents;
	
public:
	FOnSpawnCrowdsNet OnSpawnCrowdsNet;
	
private:
	uint32 GetID(uint32 typeCrowds, uint32 indexSelected);
	uint32 GetPath(FVector location, uint32 indexSelected);
	void SetAnimationData(uint32 id, uint32 typeCrowds, uint32 indexSelected);
	void SendCrowdsNetData(UCrowdsIdentity* identity, uint32 indexSelected, uint32 typeCrowds, int16 total);
	
public:
	void Initialize(uint16* crowdsActive, uint16* crowdsTotal, TArray<TObjectPtr<UInstancedStaticMeshComponent>> crowdsComponents, TMap<FName, int8> crowdsTypes, FFluxPrimeCrowds* crowdsData, FFluxPrimeNavigationSystems* navigationSystems);
	virtual void SpawnCrowd_Implementation(UCrowdsIdentity* identity, FVector location, FRotator rotation) override;
};
