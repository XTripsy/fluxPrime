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
	UPROPERTY(EditAnywhere, Category = "Crowds | Crowds Manager | Targets", meta = (AllowPrivateAccess = true))
	TArray<FFluxPrimeTargetCatalog> CrowdsTarget;
	
	FOnCrowdsManagerActionChange OnCrowdsManagerActionChange;
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> CrowdsComponents;
	
#pragma region SoftRefrence
	
	UPROPERTY(EditDefaultsOnly, Category = "Crowds | Crowds Manager | Data")
	TMap<FName, TSoftObjectPtr<UStaticMesh>> CrowdsMeshSoftRef;
	
	UPROPERTY(EditDefaultsOnly, Category = "Crowds | Crowds Manager | Data")
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>> CrowdsAnimationSoftRef;
	
	TSharedPtr<FStreamableHandle> StreamingHandle;
	
#pragma endregion 
	
	UPROPERTY()
	TMap<FName, int8> CrowdsTypes;
	
	UPROPERTY()
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
	
	inline void SpawnCrowd(UFluxPrimeCrowdsIdentity* identity, FVector location, FRotator rotation, 
		FVector randomBoundingBox, FGameplayTag newState, int32 totalEntityToSpawn)
	{
		if (!ModuleFragments || CrowdsSpawnerModuleIndex == INDEX_NONE) return;
		auto& moduleFragments = *ModuleFragments;

		TArray<FVector_NetQuantize10> listSpawnLocation;
		
		for (int i = 0; i < totalEntityToSpawn; ++i)
		{
			FVector resultLocation = UKismetMathLibrary::RandomPointInBoundingBox(location, randomBoundingBox);
			listSpawnLocation.Add(resultLocation);
			
			FFluxPrimeCrowdsSpawnerComponentContext context;
			context.crowdsTypes = &CrowdsTypes;
			context.dataStores = &DataStores;
			context.identity = identity;
			context.location = resultLocation;
			context.rotation = rotation;
			context.newState = newState;
			context.getStateID = [this](FGameplayTag Identity)
			{
				return GetStateID(Identity);
			};
		
			moduleFragments[CrowdsSpawnerModuleIndex].GetMutable<FFluxPrimeCrowdsSpawnerModule>().SpawnCrowd(context);
		}
		
		FFluxPrimeCrwodsSpawnActionPayload data;
		data.Identity = identity;
		data.Locations = listSpawnLocation;
		data.Rotation = rotation;
		data.NewState = newState;
		FInstancedStruct payload = FInstancedStruct::Make(data);
		
		OnCrowdsManagerActionChange.ExecuteIfBound(payload);
	};
	
	inline void ShootRay(FVector startRay, FVector endRay, int32 damage)
	{
		auto& moduleFragments = *ModuleFragments;
		
		int32 entityType = INDEX_NONE;
		int32 entityIndex = INDEX_NONE;
		
		moduleFragments[CrowdsSystemModuleIndex].GetMutable<FFluxPrimeCrowdsSystemsModule>().ShootRay(DataStores, startRay, endRay, entityType, entityIndex);
		
		if (entityType == INDEX_NONE || entityIndex == INDEX_NONE) return;
		
		moduleFragments[CrowdsSystemModuleIndex].GetMutable<FFluxPrimeCrowdsSystemsModule>().TakeDamage(DataStores[entityType], entityType, entityIndex, damage);
	};
};
