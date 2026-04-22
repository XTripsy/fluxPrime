// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CrowdsIdentity.generated.h"

class UFluxPrimeAnimationData;
//class UAnimToTextureDataAsset;

/**
 * 
 */
UCLASS()
class FLUXPRIME_API UCrowdsIdentity : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UFluxPrimeAnimationData> AnimationData;
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UStaticMesh> Mesh;
	
	UPROPERTY(EditAnywhere)
	FName Identity;
	
	UPROPERTY(EditAnywhere)
	int32 Speed;
	
	UPROPERTY(EditAnywhere)
	int32 Damage;
	
	UPROPERTY(EditAnywhere)
	int32 Health;
	
	UPROPERTY(EditAnywhere)
	int32 Size;
};
