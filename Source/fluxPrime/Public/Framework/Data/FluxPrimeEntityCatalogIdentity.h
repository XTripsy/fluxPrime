// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeIdentityData.h"
#include "Engine/DataAsset.h"
#include "FluxPrimeEntityCatalogIdentity.generated.h"

UCLASS()
class FLUXPRIME_API UFluxPrimeEntityCatalogIdentity : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FFluxPrimeArchetypeEntityCatalog> ArchetypeEntityCatalog;
};
