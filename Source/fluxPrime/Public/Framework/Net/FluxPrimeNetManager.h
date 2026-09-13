// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FluxPrimeNetManager.generated.h"

struct FInstancedStruct;

DECLARE_DELEGATE_OneParam(FOnNetManagerActionChange, FInstancedStruct);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLUXPRIME_API UFluxPrimeNetManager : public UActorComponent
{
	GENERATED_BODY()
	
public:
	FOnNetManagerActionChange OnCrowdsNetManagerActionChange;
	
public:
	UFluxPrimeNetManager();
	
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	UFUNCTION(Server, Reliable)
	void ServerActionChange(const FInstancedStruct& payload);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastActionChange(const FInstancedStruct& payload);
	
	void OnActionChange(FInstancedStruct payload);
};