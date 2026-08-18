// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "StructUtils/InstancedStruct.h"
#include "FluxPrimeCrowdsIdentity.generated.h"

class UFluxPrimeAnimationData;
enum class EFluxPrimeCrowdState : uint8;

USTRUCT()
struct FAbilityFragmentWrapper
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta=(BaseStruct="FluxPrimeBaseAbilitySystems", ExcludeBaseStruct))
	TObjectPtr<UScriptStruct> AbilityFragment;
	
	UPROPERTY(EditAnywhere)
	FGuid ID;
};

UCLASS(BlueprintType)
class FLUXPRIME_API UFluxPrimeCrowdsIdentity : public UPrimaryDataAsset
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
	int32 AbilityRange;
	
	UPROPERTY(EditAnywhere)
	float AggroDistance;
	
	UPROPERTY(EditAnywhere)
	FAbilityFragmentWrapper AbilityFragmentWrapper;
	
	UPROPERTY(EditAnywhere)
	float Damage;
	
	UPROPERTY(EditAnywhere)
	int32 Health;
	
	UPROPERTY(EditAnywhere)
	int32 Size;
	
};
