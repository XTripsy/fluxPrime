#pragma once

#include "CoreMinimal.h"
#include "Generics/Systems/Animation/FluxPrimeNotifyBase.h"
#include "FluxPrimeCrowdsNotifySpawnVFX.generated.h"

USTRUCT(BlueprintType)
struct FNotifySpawnVfxDataTagConfig
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
struct FFluxPrimeCrowdsNotifySpawnVFXContext
{
	GENERATED_BODY()
	
	FGetNotifyID getNotifyID = nullptr;
	//FFluxPrimeDataStore* dataStore = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsNotifySpawnVFX : public FFluxPrimeNotifyBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FNotifySpawnVfxDataTagConfig DataTagConfig;

	//int16 EntityDataRequestNotifyID;
	TArray<int16> EntityDataRequestNotifyID;
	
public:
	void Initialized(FFluxPrimeCrowdsNotifySpawnVFXContext context)
	{
		check(context.getNotifyID);
		//check(context.dataStore);
		check(context.dataStores);
		
		NotifyID = context.getNotifyID(NotifyTag);
		
		/*auto& dataStore = *context.dataStore;

		EntityDataRequestNotifyID = dataStore.GetDataID(DataTagConfig.EntityDataRequestNotifyTag);*/

		for (auto& dataStore : *context.dataStores)
		{
			EntityDataRequestNotifyID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestNotifyTag));
		}
	}
	
	//void ExecuteNotify(const uint16 indexMember, FFluxPrimeDataStore& dataStore)
	void ExecuteNotify(const uint16 indexDataID, const uint16 indexMember, FFluxPrimeArchetypeDataStore& dataStore)
	{
		//FFluxPrimeWrapperInt32* entityDataRequestNotify = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataRequestNotifyID));
		FFluxPrimeWrapperInt32* entityDataRequestNotify = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataRequestNotifyID[indexDataID]));
		if (entityDataRequestNotify[indexMember].ValueInt32 != NotifyID) return;
		
		entityDataRequestNotify[indexMember].ValueInt32 = INDEX_NONE;
		UE_LOG(LogTemp, Log, TEXT("NOTIFY:: SPAWN VFX"));
	}
};
