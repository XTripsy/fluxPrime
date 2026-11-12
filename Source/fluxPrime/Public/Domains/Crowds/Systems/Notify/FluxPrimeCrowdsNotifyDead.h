#pragma once

#include "CoreMinimal.h"
#include "Generics/Systems/Animation/FluxPrimeNotifyBase.h"
#include "FluxPrimeCrowdsNotifyDead.generated.h"

USTRUCT(BlueprintType)
struct FNotifyDeadDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRequestNotifyConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLocationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRequestBackToPoolConfig;

	bool IsValid() const
	{
		return EntityDataRequestNotifyConfig.EntityDataTag.IsValid() 
		   && EntityDataLocationConfig.EntityDataTag.IsValid() 
		   && EntityDataRequestBackToPoolConfig.EntityDataTag.IsValid();
	}
};

USTRUCT()
struct FFluxPrimeCrowdsNotifyDeadContext
{
	GENERATED_BODY()
	
	FGetNotifyID getNotifyID = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsNotifyDead : public FFluxPrimeNotifyBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FNotifyDeadDataTagConfig DataTagConfig;

	TArray<int16> EntityDataRequestNotifyID, EntityDataLocationID, EntityDataRequestBackToPoolID;
	
public:
	void Initialized(FFluxPrimeCrowdsNotifyDeadContext context)
	{
		check(context.getNotifyID);
		check(context.dataStores);
		
		int32 countData = context.dataStores->Num();
		EntityDataRequestNotifyID.Init(0, countData);
		EntityDataLocationID.Init(0, countData);
		EntityDataRequestBackToPoolID.Init(0, countData);
		
		NotifyID = context.getNotifyID(NotifyTag);
		
		int32 indexDataId = 0;
		for (auto& dataStore : *context.dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataRequestNotifyConfig, EntityDataRequestNotifyID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataLocationConfig, EntityDataLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRequestBackToPoolConfig, EntityDataRequestBackToPoolID[indexDataId]);
			indexDataId++;
		}
	}
	
	void ExecuteNotify(const uint16 indexDataID, const uint16 indexMember, FFluxPrimeArchetypeDataStore& dataStore)
	{
		if (!IsAllDataValid)
		{
			UE_LOG(LogTemp, Error, TEXT("NOTIFY DEAD :: SOME DATA NOT VALID"));
			return;
		}
		
		FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
		FFluxPrimeWrapperInt32* entityDataRequestNotify = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataRequestNotifyID[indexDataID]));
		FFluxPrimeWrapperBool* entityDataRequestBackToPool = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataRequestBackToPoolID[indexDataID]));
		
		check(entityDataLocation);
		check(entityDataRequestNotify);
		check(entityDataRequestBackToPool);
		
		if (entityDataRequestNotify[indexMember].ValueInt32 != NotifyID) return;
		
		entityDataLocation[indexMember] = FVector::DownVector * 1000.0f;
		entityDataRequestBackToPool[indexMember].ValueBool = true;
		entityDataRequestNotify[indexMember].ValueInt32 = INDEX_NONE;
	}
};
