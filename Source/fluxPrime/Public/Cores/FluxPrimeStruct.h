// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "FluxPrimeStruct.generated.h"

class UCrowdsIdentity;
class IFluxPrimeCrowdLeaderInterface;

USTRUCT(BlueprintType)
struct FFluxCrowdsPath
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector LocationPaths[8];
};

USTRUCT(BlueprintType)
struct FFluxPrimeMemberNet
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FVector_NetQuantize100 NetLocation;
	
	UPROPERTY(EditAnywhere)
	int8 NetRotation;
	
	UPROPERTY(EditAnywhere)
	int8 NetType;
	
	UPROPERTY(EditAnywhere)
	bool isActive;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowds
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsLocation;
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsVelocity;
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsAcceleration;
	
	UPROPERTY(EditAnywhere)
	TArray<FFluxCrowdsPath> CrowdsNavigationPath;
	
	UPROPERTY(EditAnywhere)
	TArray<FVector> CrowdsTargetLocation;
	
	UPROPERTY(EditAnywhere)
	TArray<int32> CrowdsCellID;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsRotation;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsMaxSpeed;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsType;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsID;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsDamage;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsHealth;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsSize;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsIndexNavigationPath;
	
	UPROPERTY(EditAnywhere)
	TArray<int8> CrowdsTotalNavigationPath;
	
	void Init(int32 totalMember)
	{
		CrowdsLocation.Reserve(totalMember);
		CrowdsVelocity.Reserve(totalMember);
		CrowdsAcceleration.Reserve(totalMember);
		CrowdsNavigationPath.Reserve(totalMember);
		CrowdsTargetLocation.Reserve(totalMember);
		CrowdsCellID.Reserve(totalMember);
		CrowdsRotation.Reserve(totalMember);
		CrowdsMaxSpeed.Reserve(totalMember);
		CrowdsType.Reserve(totalMember);
		CrowdsID.Reserve(totalMember);
		CrowdsHealth.Reserve(totalMember);
		CrowdsDamage.Reserve(totalMember);
		CrowdsSize.Reserve(totalMember);
		CrowdsIndexNavigationPath.Reserve(totalMember);
		CrowdsTotalNavigationPath.Reserve(totalMember);
	}
};

USTRUCT(BlueprintType)
struct FFluxCatalogCrowds
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	TObjectPtr<UCrowdsIdentity> CrowdsIdentity;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	int32 CrowdsTotal;
};
