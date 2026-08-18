// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeEvent.h"
#include "Engine/StreamableManager.h"
#include "Components/ActorComponent.h"
#include "Crowds/Modules/FluxPrimeCrowdsDataModule.h"
#include "Crowds/Modules/FluxPrimeCrowdsSpawnerModule.h"
#include "Crowds/Modules/FluxPrimeCrowdsSystemsModule.h"
#include "FluxPrimeCrowdsManager.generated.h"

USTRUCT()
struct FFluxPrimeCrowdsManagerContext
{
	GENERATED_BODY()
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* CrowdsComponents;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), HideCategories=(Tags, Activation, Cooking, AssetUserData, Navigation))
class FLUXPRIME_API UFluxPrimeCrowdsManager : public UActorComponent
{
	GENERATED_BODY()

public:
#pragma region Configuration
	
	UPROPERTY(EditAnywhere, Category = "Crowds | Crowds Manager | CollisionProfile", meta = (AllowPrivateAccess = true))
	FName ProfileNameCollisionCrowds = "Pawn";
	
	UPROPERTY(EditAnywhere, Category = "Crowds | Crowds Manager | Condition", meta = (AllowPrivateAccess = true))
	bool IsShowDebug;
	
	UPROPERTY(EditAnywhere, Category = "Crowds | Crowds Manager | Configuration", meta = (AllowPrivateAccess = true))
	TObjectPtr<UManagerConfiguration> ManagerConfiguration;
	
	UPROPERTY(EditAnywhere, Category = "Crowds | Crowds Manager | Catalogs", meta = (AllowPrivateAccess = true))
	TArray<FFluxPrimeCrowdsCatalog> CrowdsCatalog;
	
	UPROPERTY(EditAnywhere, Category = "Crowds | Crowds Manager | Targets", meta = (AllowPrivateAccess = true))
	TArray<FFluxPrimeTargetCatalog> CrowdsTarget;
	
#pragma endregion 
	
	FOnCrowdsManagerActionChange OnCrowdsManagerActionChange;
	
private:
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* CrowdsComponents;
	
#pragma region SoftRefrence
	
	UPROPERTY(EditDefaultsOnly, Category = "Crowds | Crowds Manager | Data")
	TMap<FName, TSoftObjectPtr<UStaticMesh>> CrowdsMeshSoftRef;
	
	UPROPERTY(EditDefaultsOnly, Category = "Crowds | Crowds Manager | Data")
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>> CrowdsAnimationSoftRef;
	
	TSharedPtr<FStreamableHandle> StreamingHandle;
	
#pragma endregion 
	
	UPROPERTY()
	TMap<FName, int8> CrowdsTypes;
	
	TArray<TArray<int16>> CrowdsPool;
	
	UPROPERTY()
	TArray<int16> CrowdsHeadPool;
	
	UPROPERTY()
	uint16 CrowdsTotal = 0;
	
	UPROPERTY()
	uint16 CrowdsActive = 0;
	
	UPROPERTY()
	FFluxPrimeCrowds CrowdsDatas;
	
	UPROPERTY()
	TMap<FFluxPrimeCrowdsLookup, int32> CrowdsLookup;
	
#pragma region Module
	
	UPROPERTY()
	FFluxPrimeCrowdsDataModule CrowdsDataModule;
	
	UPROPERTY()
	FFluxPrimeCrowdsSpawnerModule CrowdsSpawnerModule;
	
	UPROPERTY()
	FFluxPrimeCrowdsSystemsModule CrowdsSystemsModule; 
	
#pragma endregion
	
public:
	UFluxPrimeCrowdsManager();
	void InitializeManager(FFluxPrimeCrowdsManagerContext context);
	void PreLoading();

private:
	void ShowDebug();
	
	UFUNCTION()
	void Initialize();
	
	void InitializeComponentCrowds();
	void InitializedComponentSystems();
	void InitializedPlayer();
	
protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	void OnActionChange(FInstancedStruct payload);
	
	UFUNCTION(BlueprintCallable)
	inline void SpawnCrowd(UFluxPrimeCrowdsIdentity* identity, FVector location, FRotator rotation)
	{
		CrowdsSpawnerModule.SpawnCrowd(identity, location, rotation);
		
		FFluxPrimeSpawnActionPayload data;
		data.Identity = identity;
		data.Location = location;
		data.Rotation = rotation;
		FInstancedStruct payload = FInstancedStruct::Make(data);
		
		OnCrowdsManagerActionChange.ExecuteIfBound(payload);
	};
	
	UFUNCTION(BlueprintCallable)
	inline void ChangeState(EFluxPrimeCrowdState newState)
	{
		for (int i = 0; i < CrowdsActive; ++i)
		{
			CrowdsDatas.CrowdsState[i] = newState;
		}
	};
	
	UFUNCTION(BlueprintCallable)
	inline void TakeDamage(int32 id, FName type)
	{
		FFluxPrimeCrowdsLookup lookup;
		lookup.CrowdsID = id;
		lookup.CrowdsType = *CrowdsTypes.Find(type);
		
		uint32 index = *CrowdsLookup.Find(lookup);
		
		if (index == INDEX_NONE) return;
		
		UE_LOG(LogTemp, Log, TEXT("LOOKUP:: %d"), index);
		CrowdsSystemsModule.Damage(index);
		
		FFluxPrimeDamageActionPayload data;
		data.CrowdID = id;
		data.CrowdType = lookup.CrowdsType;
		FInstancedStruct payload = FInstancedStruct::Make(data);
		
		OnCrowdsManagerActionChange.ExecuteIfBound(payload);
	};
};
