// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Domains/Crowds/Event/FluxPrimeCrowdsEvent.h"
#include "Engine/StreamableManager.h"
#include "Components/ActorComponent.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsData.h"
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
	void RequestAction(FInstancedStruct payload);
	void OnActionChange(FInstancedStruct payload);
	
	inline TMap<FUniqueNetIdRepl, int32> GetPlayerKillsData() const { return PlayerKillsData; }
};
