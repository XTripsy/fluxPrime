// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Cores/FluxPrimeAnimationData.h"
#include "Cores/FluxPrimeStruct.h"
#include "Crowds/Identity/CrowdsIdentity.h"
#include "FluxPrimeCrowdsDataComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLUXPRIME_API UFluxPrimeCrowdsDataComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CrowdsComponents;
	
	UPROPERTY()
	TArray<FFluxPrimeCrowdsCatalog> CrowdsCatalog;
	
	UPROPERTY()
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>> CrowdsAnimationSoftRef;
	
	uint16* CrowdsTotal = 0;
	int8* CrowdsDataReadIndex = 0;
	
	UPROPERTY()
	TArray<int32> CrowdsDataShortedIndex;
	
	UPROPERTY()
	TArray<FVector_NetQuantize100> NetAcceleration;
	
	UPROPERTY()
	TArray<FVector_NetQuantize100> NetTarget;
	
	TStaticArray<FFluxPrimeCrowds, 2>* CrowdsDatas;
	
	bool bHasAuthority, bIsReplicated;
	
public:
	UFluxPrimeCrowdsDataComponent();
	/*void Initialize(FFluxPrimeCrowdsDataComponentContext context);
	void InitializeCrowds();*/
};
