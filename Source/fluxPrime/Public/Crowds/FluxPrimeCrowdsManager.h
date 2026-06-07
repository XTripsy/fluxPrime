// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
#include "FluxPrimeCrowdsManagerInterface.h"
#include "Cores/FluxPrimeStruct.h"
#include "Modules/FluxPrimeCrowdsAnimationModule.h"
#include "Modules/FluxPrimeCrowdsDataModule.h"
#include "Modules/FluxPrimeCrowdsSpawnerModule.h"
#include "Modules/FluxPrimeCrowdsSystemsModule.h"
#include "Systems/States/FluxPrimeAbilitySystems.h"
#include "Systems/FluxPrimeBoidsSystems.h"
#include "Systems/FluxPrimeGroundHeightSystems.h"
#include "Systems/FluxPrimeMovementSystems.h"
#include "Systems/FluxPrimeNavigationSystems.h"
#include "Systems/FluxPrimeSpatialGridSystems.h"
#include "Systems/FluxPrimeAnimationSystems.h"
#include "Systems/FluxPrimeCrowdsRenderSystems.h"
#include "Systems/FluxPrimeDamageSystems.h"
#include "Systems/States/FluxPrimeIdleSystems.h"
#include "Systems/FluxPrimeProxyTargetSystems.h"
#include "Systems/FluxPrimeStateMachineSystems.h"
#include "Systems/States/FluxPrimeWalkSystems.h"
#include "FluxPrimeCrowdsManager.generated.h"

class UFluxPrimeCrowdsNetComponent;
class UManagerConfiguration;

UCLASS(NotBlueprintable, HideCategories=(Rendering, Replication, Collision, Input, 
		Actor, LOD, Cooking, Transform, Physics, Networking, LevelInstance, HLOD, WorldPartition, DataLayers))
class FLUXPRIME_API AFluxPrimeCrowdsManager final : public AActor, public IFluxPrimeCrowdsManagerInterface
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CrowdsComponents;
	
	UPROPERTY()
	TObjectPtr<UFluxPrimeCrowdsNetComponent> CrowdsNetComponent;
	
private:
#pragma region Configuration
	
	UPROPERTY(EditAnywhere, Category = "Crowds | CollisionProfile", meta = (AllowPrivateAccess = true))
	FName ProfileNameCollisionCrowds = "Pawn";
	
	UPROPERTY(EditAnywhere, Category = "Crowds | Condition", meta = (AllowPrivateAccess = true))
	bool IsReplicated;
	
	UPROPERTY(EditAnywhere, Category = "Crowds | Condition", meta = (AllowPrivateAccess = true))
	bool IsShowDebug;
	
	UPROPERTY(EditAnywhere, Category = "Crowds | Configuration", meta = (AllowPrivateAccess = true))
	TObjectPtr<UManagerConfiguration> ManagerConfiguration;
	
	UPROPERTY(EditAnywhere, Category = "Crowds | Catalogs", meta = (AllowPrivateAccess = true))
	TArray<FFluxPrimeCrowdsCatalog> CrowdsCatalog;
	
#pragma endregion 
	
#pragma region SoftRefrence
	
	UPROPERTY(EditDefaultsOnly, Category = "Crowds | Data")
	TMap<FName, TSoftObjectPtr<UStaticMesh>> CrowdsMeshSoftRef;
	
	UPROPERTY(EditDefaultsOnly, Category = "Crowds | Data")
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>> CrowdsAnimationSoftRef;
	
	TSharedPtr<FStreamableHandle> StreamingHandle;
	
#pragma endregion 
	
	UPROPERTY()
	TMap<FName, int8> CrowdsTypes;
	
	UPROPERTY()
	int8 CrowdsDataReadIndex = 0;
	
	UPROPERTY()
	TArray<int32> CrowdsDataShortedIndex;
	
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
	
	UPROPERTY()
	TArray<int32> GridOffset;
	
	TStaticArray<FFluxPrimeCrowds, 2> CrowdsDatas;
	
#pragma region Module
	
	UPROPERTY()
	FFluxPrimeCrowdsDataModule CrowdsDataModule;
	
	UPROPERTY()
	FFluxPrimeCrowdsSpawnerModule CrowdsSpawnerModule;
	
	UPROPERTY()
	FFluxPrimeCrowdsAnimationModule CrowdsAnimationModule;
	
	UPROPERTY()
	FFluxPrimeCrowdsSystemsModule CrowdsSystemsModule; 
	
#pragma endregion

#pragma region Systems
	
	UPROPERTY()
	FFluxPrimeCrowdsRenderSystems CrowdsRenderSystems;
	
	UPROPERTY()
	FFluxPrimeSpatialGridSystems SpatialGridSystems;
	
	UPROPERTY()
	FFluxPrimeBoidsSystems BoidsSystems;
	
	UPROPERTY()
	FFluxPrimeMovementSystems MovementSystems;
	
	UPROPERTY()
	FFluxPrimeNavigationSystems NavigationSystems;
	
	UPROPERTY()
	FFluxPrimeGroundHeightSystems GroundHeightSystems;
	
	UPROPERTY()
	FFluxPrimeAnimationSystems AnimationSystems; 
	
	UPROPERTY()
	FFluxPrimeProxyTargetSystems ProxyTargetSystems;
	
	UPROPERTY()
	FFluxPrimeDamageSystems DamageSystems;
	
	UPROPERTY()
	FFluxPrimeStateMachineSystems StateMachineSystems;
	
#pragma endregion

	
public:
	AFluxPrimeCrowdsManager();

private:
	void ShowDebug();
	
	void ShortCrowdsByID();
	
	void PreLoading();
	
	UFUNCTION()
	void Initialize();
	
	void InitializeComponentCrowds();
	void InitializedComponentSystems();
	
	UFUNCTION()
	void OnRep_CrowdActive();
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SpawnCrowd(UCrowdsIdentity* identity, FVector location, FRotator rotation)
	{
		CrowdsSpawnerModule.SpawnCrowd(identity, location, rotation);
	};
	
	// temp
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void ChangeState()
	{
		for (int i = 0; i < CrowdsActive; ++i)
		{
			CrowdsDatas[CrowdsDataReadIndex].CrowdsState[i] = EFluxPrimeCrowdState::StateWalk;
		}
	};
	
public:
	virtual void TakeDamage_Implementation(UCrowdsIdentity* Identity) override;
};
