// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Net/UnrealNetwork.h"
#include "Components/ActorComponent.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeCrowdsNetComponent.generated.h"

struct FFluxPrimeCrowdsRenderSystems;
struct FFluxPrimeMovementSystems;
struct FFluxPrimeGroundHeightSystems;
struct FFluxPrimeCrowdsAnimationNet;
struct FInstancedStruct;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLUXPRIME_API UFluxPrimeCrowdsNetComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CrowdsComponents;
	
	UPROPERTY()
	uint16 CrowdsActive;
	
	UPROPERTY()
	uint16 CrowdsTotal;
	
	//UPROPERTY(ReplicatedUsing = OnRep_CrowdData)
	UPROPERTY(Replicated)
	TArray<FFluxPrimeCrowdsNet> CrowdsNets;
	
	UPROPERTY()
	TArray<FFluxPrimeCrowdsAccelerationNet> CrowdsAccelerationsNets;
	
	UPROPERTY()
	TArray<FFluxPrimeCrowdsTargetNet> CrowdsTargetNets;
	//TArray<FVector> CrowdsAccelerationsNets;
	
	UPROPERTY(ReplicatedUsing = OnRep_CrowdDataAnimation)
	TArray<FFluxPrimeCrowdsAnimationNet> CrowdsAnimationNets;
	
public:
	UFluxPrimeCrowdsNetComponent();
	
private:
	/*UFUNCTION()
	void OnRep_CrowdData();*/
	
	UFUNCTION()
	void OnRep_CrowdDataAnimation();
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	void Initialize(uint16 crowdsTotal, TArray<TObjectPtr<UInstancedStaticMeshComponent>> crowdsComponents);
	void UpdateNetData(float DeltaTime, FFluxPrimeGroundHeightSystems& GroundHeightSystems, FFluxPrimeMovementSystems& MovementSystems, FFluxPrimeCrowdsRenderSystems& RenderSystems);
	void UpdateCrowdsData(const TArray<FVector_NetQuantize100>& accelerations, const TArray<FVector_NetQuantize100>& target);
	void OnCrowdsActiveChange(uint16 count);
	
	UFUNCTION()
	void OnSpawnCrowdsData(const FInstancedStruct& payload);
};
