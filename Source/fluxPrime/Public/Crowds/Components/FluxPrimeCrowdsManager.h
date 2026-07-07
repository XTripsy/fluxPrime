// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsNetComponent> CrowdsNetComponent;
	
	UPROPERTY()
	bool isReplicated;
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
	
#pragma endregion 
	
private:
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* CrowdsComponents;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsNetComponent> CrowdsNetComponent;
	
	UPROPERTY()
	bool IsReplicated;
	
#pragma region SoftRefrence
	
	UPROPERTY(EditDefaultsOnly, Category = "Crowds | Crowds Manager | Data")
	TMap<FName, TSoftObjectPtr<UStaticMesh>> CrowdsMeshSoftRef;
	
	UPROPERTY(EditDefaultsOnly, Category = "Crowds | Crowds Manager | Data")
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>> CrowdsAnimationSoftRef;
	
	TSharedPtr<FStreamableHandle> StreamingHandle;
	
#pragma endregion 
	
	UPROPERTY()
	TMap<FName, int8> CrowdsTypes;
	
	/*UPROPERTY()
	int8 CrowdsDataReadIndex = 0;
	
	UPROPERTY()
	TArray<int32> CrowdsDataShortedIndex;*/
	
	UPROPERTY()
	uint16 CrowdsTotal = 0;
	
	UPROPERTY(ReplicatedUsing = OnRep_CrowdActive)
	uint16 CrowdsActive = 0;
	
	// variable ini sering di kirim ke clinet 
	UPROPERTY(Replicated)
	TArray<FVector_NetQuantize100> NetAcceleration;
	
	// variable ini sering di kirim ke clinet 
	UPROPERTY(Replicated)
	TArray<FVector_NetQuantize100> NetTarget;
	
	/*UPROPERTY()
	TArray<int32> GridOffset;*/
	
	//TStaticArray<FFluxPrimeCrowds, 2> CrowdsDatas;
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
	void ShortCrowdsByID();
	
	UFUNCTION()
	void Initialize();
	
	void InitializeComponentCrowds();
	void InitializedComponentSystems();
	
	UFUNCTION()
	void OnRep_CrowdActive();
	
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	UFUNCTION(BlueprintCallable)
	inline void SpawnCrowd(UCrowdsIdentity* identity, FVector location, FRotator rotation)
	{
		CrowdsSpawnerModule.SpawnCrowd(identity, location, rotation);
	};
	
	UFUNCTION(BlueprintCallable)
	inline void ChangeState(EFluxPrimeCrowdState newState)
	{
		for (int i = 0; i < CrowdsActive; ++i)
		{
			//CrowdsDatas[CrowdsDataReadIndex].CrowdsState[i] = newState;
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
		UE_LOG(LogTemp, Log, TEXT("LOOKUP:: %d"), index);
		
		CrowdsSystemsModule.Damage(index);
	};
};
