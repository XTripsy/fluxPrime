// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Domains/Crowds/Event/FluxPrimeCrowdsEvent.h"
#include "Engine/StreamableManager.h"
#include "Components/ActorComponent.h"
#include "Framework/Core/FluxPrimeBaseManager.h"
#include "Domains/Crowds/Modules/FluxPrimeCrowdsSystemsModule.h"
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

	/** Catalog entries resolved once at initialization. Archetype indices are positional. */
	UPROPERTY()
	TArray<TObjectPtr<UFluxPrimeCrowdsIdentity>> ResolvedCrowdIdentities;

	/** One validated animation asset per archetype, in the same order as the catalog. */
	UPROPERTY()
	TArray<TObjectPtr<UFluxPrimeAnimationData>> ResolvedAnimationData;
	
	int16 CrowdsSpawnerModuleIndex = INDEX_NONE, CrowdsSystemModuleIndex = INDEX_NONE, CrowdsDataModuleIndex = INDEX_NONE;
	
public:
	UFluxPrimeCrowdsManager();
	void PreLoading();

	/** Resolves a cached module index to the expected module type, or null when absent/mismatched. */
	template<typename T>
	T* FindModule(int32 cachedIndex) const
	{
		if (!ModuleFragments || !ModuleFragments->IsValidIndex(cachedIndex)) return nullptr;
		return (*ModuleFragments)[cachedIndex].GetMutablePtr<T>();
	}

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
	/** Module configuration owned by the duplicated manager configuration. */
	TArray<FInstancedStruct>* ModuleFragments = nullptr;

	/** Validates and resolves the catalog before any asset is loaded. Exposed for tests and subclasses. */
	bool ValidateAndResolveConfiguration();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
public:
	void RequestAction(const FInstancedStruct& payload);
	void OnActionChange(const FInstancedStruct& payload);
	
	inline const TMap<FUniqueNetIdRepl, int32>& GetPlayerKillsData() const { return PlayerKillsData; }
	inline const TArray<TObjectPtr<UFluxPrimeCrowdsIdentity>>& GetResolvedCrowdIdentities() const { return ResolvedCrowdIdentities; }
};
