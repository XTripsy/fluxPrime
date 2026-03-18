// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FluxPrimePawn.generated.h"

UCLASS()
class FLUXPRIME_API AFluxPrimePawn : public APawn
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "FluxPrime", meta = (DisplayPriority = "0"))
	bool IsUseTick;
	
public:
	AFluxPrimePawn();

protected:
	virtual void PostInitProperties() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
