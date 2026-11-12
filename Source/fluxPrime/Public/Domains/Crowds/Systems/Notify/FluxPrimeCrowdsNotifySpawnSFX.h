#pragma once

#include "CoreMinimal.h"
#include "Generics/Systems/Animation/FluxPrimeNotifyBase.h"
#include "FluxPrimeCrowdsNotifySpawnSFX.generated.h"

USTRUCT(BlueprintType)
struct FNotifySpawnSfxDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRequestNotifyConfig;

	bool IsValid() const
	{
		return EntityDataRequestNotifyConfig.EntityDataTag.IsValid();
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
		
		EntityDataRequestNotifyID.Init(0, context.dataStores->Num());
		
		World = context.world;
		NotifyID = context.getNotifyID(NotifyTag);

		int32 indexDataId = 0;
		for (auto& dataStore : *context.dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataRequestNotifyConfig, EntityDataRequestNotifyID[indexDataId]);
			indexDataId++;
		}
	}
	
	void ExecuteNotify(const uint16 indexDataID, const uint16 indexMember, FFluxPrimeArchetypeDataStore& dataStore)
	{
		if (!IsAllDataValid)
		{
			UE_LOG(LogTemp, Error, TEXT("NOTIFY SPAWN SFX :: SOME DATA NOT VALID"));
			return;
		}
		
		FFluxPrimeWrapperInt32* entityDataRequestNotify = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataRequestNotifyID[indexDataID]));
		
		check(entityDataRequestNotify);
		
		if (entityDataRequestNotify[indexMember].ValueInt32 != NotifyID) return;
		
		entityDataRequestNotify[indexMember].ValueInt32 = INDEX_NONE;
	}
};
