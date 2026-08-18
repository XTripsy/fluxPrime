// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Cores/FluxPrimeEvent.h"
#include "FluxPrimeCrowdsNetManager.generated.h"

struct FFluxPrimeRenderSystem;
struct FFluxPrimeMovementSystem;
struct FFluxPrimeGroundHeightSystem;
struct FFluxPrimeCrowdsAnimationNet;
struct FInstancedStruct;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLUXPRIME_API UFluxPrimeCrowdsNetManager : public UActorComponent
{
	GENERATED_BODY()
	
public:
	FOnCrowdsNetManagerActionChange OnCrowdsNetManagerActionChange;
	
public:
	UFluxPrimeCrowdsNetManager();
	
public:
	UFUNCTION(Server, Reliable)
	void ServerActionChange(const FInstancedStruct& payload);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastActionChange(const FInstancedStruct& payload);
	
	void OnActionChange(FInstancedStruct payload);
};
