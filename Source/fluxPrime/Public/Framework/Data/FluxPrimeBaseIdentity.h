// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeStateIdentity.h"
#include "Engine/DataAsset.h"
#include "FluxPrimeBaseIdentity.generated.h"

UCLASS(Abstract)
class FLUXPRIME_API UFluxPrimeBaseIdentity : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UStaticMesh> Mesh;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UFluxPrimeStateIdentity> StateIdentity;
	
	UPROPERTY(EditAnywhere)
	FName Identity;
	
	UPROPERTY(EditAnywhere)
	int32 Size;
};
