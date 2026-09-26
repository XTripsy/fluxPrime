#pragma once

#include "CoreMinimal.h"
#include "Generics/Systems/Animation/FluxPrimeNotifyBase.h"
#include "FluxPrimeCrowdsNotifySpawnSFX.generated.h"

USTRUCT(BlueprintType)
struct FNotifySpawnSfxDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRequestNotifyTag;

	bool IsValid() const
	{
		return EntityDataRequestNotifyTag.IsValid();
	}
};

USTRUCT()
struct FFluxPrimeCrowdsNotifySpawnSFXContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world = nullptr;
	
	FGetNotifyID getNotifyID = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsNotifySpawnSFX : public FFluxPrimeNotifyBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FNotifySpawnSfxDataTagConfig DataTagConfig;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> SFXSystem = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;

	TArray<int16> EntityDataRequestNotifyID;
	
public:
	void Initialized(FFluxPrimeCrowdsNotifySpawnSFXContext context)
	{
		check(context.world);
		check(context.getNotifyID);
		check(context.dataStores);
		
		World = context.world;
		NotifyID = context.getNotifyID(NotifyTag);

		for (auto& dataStore : *context.dataStores)
		{
			EntityDataRequestNotifyID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestNotifyTag));
		}
	}
	
	void ExecuteNotify(const uint16 indexDataID, const uint16 indexMember, FFluxPrimeArchetypeDataStore& dataStore)
	{
		FFluxPrimeWrapperInt32* entityDataRequestNotify = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataRequestNotifyID[indexDataID]));
		if (entityDataRequestNotify[indexMember].ValueInt32 != NotifyID) return;
		
		entityDataRequestNotify[indexMember].ValueInt32 = INDEX_NONE;
		UE_LOG(LogTemp, Log, TEXT("NOTIFY:: SPAWN SFX"));
	}
};
