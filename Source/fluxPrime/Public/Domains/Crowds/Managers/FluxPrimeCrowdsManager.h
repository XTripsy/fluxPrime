// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Domains/Crowds/Event/FluxPrimeCrowdsEvent.h"
#include "Engine/StreamableManager.h"
#include "Components/ActorComponent.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsData.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsPayloadData.h"
#include "Framework/Core/FluxPrimeBaseManager.h"
#include "Domains/Crowds/Modules/FluxPrimeCrowdsSpawnerModule.h"
#include "Domains/Crowds/Modules/FluxPrimeCrowdsSystemsModule.h"
#include "Kismet/KismetMathLibrary.h"
#include "FluxPrimeCrowdsManager.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), HideCategories=(Tags, Activation, Cooking, AssetUserData, Navigation))
class FLUXPRIME_API UFluxPrimeCrowdsManager : public UFluxPrimeBaseManager
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Crowds | Crowds Manager | Targets", meta = (AllowPrivateAccess = true))
	TArray<FFluxPrimeTargetCatalog> CrowdsTarget;
	
	FOnCrowdsManagerActionChange OnCrowdsManagerActionChange;
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CrowdsComponents;
	
#pragma region SoftRefrence
	
	TMap<FName, TSoftObjectPtr<UStaticMesh>> CrowdsMeshSoftRef;
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>> CrowdsAnimationSoftRef;
	TSharedPtr<FStreamableHandle> StreamingHandle;
	
#pragma endregion 
	
	UPROPERTY(VisibleAnywhere)
	TMap<FUniqueNetIdRepl, int32> PlayerKillsData;
	TMap<FName, int8> CrowdsTypes;
	TArray<FGameplayTag> NotifyID;
	
	TArray<FInstancedStruct>* ModuleFragments;
	int16 CrowdsSpawnerModuleIndex = INDEX_NONE, CrowdsSystemModuleIndex = INDEX_NONE, CrowdsDataModuleIndex = INDEX_NONE;
	
public:
	UFluxPrimeCrowdsManager();
	void PreLoading();

private:
	UFUNCTION()
	void Initialize();
	
	void InitializedModules();
	void InitializeComponentCrowds();
	
	UFUNCTION(BlueprintCallable)
	void InitializedPlayer();
	
	inline void RegisterNotifyID(FGameplayTag identity)
	{
		if (GetNotifyID(identity) != INDEX_NONE) return;

		NotifyID.Add(identity);
	}
	
	inline int32 GetNotifyID(FGameplayTag identity) const
	{
		int32 index = 0;
		for (const auto& pair : NotifyID)
		{
			if (pair == identity) return index;
			index++;
		}

		return INDEX_NONE;
	}
	
protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	void OnActionChange(FInstancedStruct payload);
	
	inline TMap<FUniqueNetIdRepl, int32> GetPlayerKillsData() const { return PlayerKillsData; }
	
	inline void SpawnCrowd(UFluxPrimeCrowdsIdentity* identity, FVector location, FRotator rotation, 
		FVector randomBoundingBox, FGameplayTag newState, int32 totalEntityToSpawn)
	{
		if (!ModuleFragments || !ModuleFragments->IsValidIndex(CrowdsSpawnerModuleIndex)) return;

		auto& spawnerModule = (*ModuleFragments)[CrowdsSpawnerModuleIndex].GetMutable<FFluxPrimeCrowdsSpawnerModule>();

		FFluxPrimeCrowdsSpawnerComponentContext context;
		context.crowdsTypes = &CrowdsTypes;
		context.dataStores = &DataStores;
		context.identity = identity;
		context.rotation = rotation;
		context.newState = newState;
		context.getStateID = [this](FGameplayTag Identity)
		{
			return GetStateID(Identity);
		};

		TArray<FVector_NetQuantize10> listSpawnLocation;
		listSpawnLocation.Reserve(totalEntityToSpawn);
	
		for (int i = 0; i < totalEntityToSpawn; ++i)
		{
			FVector resultLocation = UKismetMathLibrary::RandomPointInBoundingBox(location, randomBoundingBox);
			listSpawnLocation.Add(resultLocation);
		
			context.location = resultLocation;
			spawnerModule.SpawnCrowd(context);
		}
	
		FFluxPrimeCrwodsSpawnActionPayload data;
		data.Identity = identity;
		data.Locations = MoveTemp(listSpawnLocation);
		data.Rotation = rotation;
		data.NewState = newState;
	
		OnCrowdsManagerActionChange.ExecuteIfBound(FInstancedStruct::Make(data));
	};
	
	inline void ShootRay(FVector startRay, FVector endRay, int32 damage, FUniqueNetIdRepl playerId)
	{
		auto& systemsModule = (*ModuleFragments)[CrowdsSystemModuleIndex].GetMutable<FFluxPrimeCrowdsSystemsModule>();

		auto* raycastSystem = systemsModule.GetSystem<FFluxPrimeRaycastSystem>();
		auto* healthSystem = systemsModule.GetSystem<FFluxPrimeHealthSystem>();
		if (!raycastSystem || !healthSystem) return;
		
		int32 entityType = INDEX_NONE;
		int32 entityIndex = INDEX_NONE;
		
		raycastSystem->ShootRay(DataStores, startRay, endRay, entityType, entityIndex);
		if (entityType == INDEX_NONE || entityIndex == INDEX_NONE) return;

		healthSystem->TakeDamage(DataStores[entityType], entityType, entityIndex, damage);

		auto* killEventSystem = systemsModule.GetSystem<FFluxPrimeCrowdsKillEventSystem>();
		if (!killEventSystem) return;
		
		if (killEventSystem->CekKillEvent(DataStores[entityType], entityType, entityIndex, damage))
		{
			int32& valueKill = PlayerKillsData.FindOrAdd(playerId);
			valueKill++;

			FFluxPrimeCrowdsKillDataPayload data;
			data.PlayerId = playerId;
			data.PlayerKillCount = valueKill;
            
			OnCrowdsManagerActionChange.ExecuteIfBound(FInstancedStruct::Make(data));
		}
	};
	
	inline void UpdatePrioriyTarget(FUniqueNetIdRepl playerID, int32 newPriority)
	{
		for (auto& pair : CrowdsTarget)
		{
			if (pair.CrowdsTargetID != playerID) continue;
			
			pair.CrowdsTargetPriority = newPriority;
		}
	}
};
