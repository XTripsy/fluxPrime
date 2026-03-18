// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimePawn.h"
#include "FluxPrimeCrowd.generated.h"

class UCapsuleComponent;
class UInstancedStaticMeshComponent;
struct FFluxPrimeNet;

UCLASS()
class FLUXPRIME_API AFluxPrimeCrowd : public AFluxPrimePawn
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> MemberComponents;

	UPROPERTY(ReplicatedUsing = OnRep_MemberData)
	TArray<int32> MemberTypeCounts;
	
	UPROPERTY(ReplicatedUsing = OnRep_MemberData)
	TArray<FFluxPrimeNet> MemberTransforms;
	
protected:
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UStaticMesh>> MemberMeshes;
	
	UPROPERTY(EditAnywhere)
	int32 TotalMemebers;
	
private:
	UFUNCTION()
	void OnRep_MemberData();
	
	UFUNCTION(BlueprintCallable)
	void DisableMemeber(FString type, int32 id);
	
	TArray<int32> SplitDynamic(int32 Total, int32 NumGroups);
	
public:
	AFluxPrimeCrowd();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
};
