#pragma once

#include "CoreMinimal.h"
#include "Generics/Systems/Animation/FluxPrimeNotifyBase.h"
#include "FluxPrimeCrowdsNotifyDead.generated.h"

USTRUCT(BlueprintType)
struct FNotifyDeadDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRequestNotifyTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRequestBackToPoolTag;

	bool IsValid() const
	{
		return EntityDataRequestNotifyTag.IsValid() 
		   && EntityDataLocationTag.IsValid() 
		   && EntityDataRequestBackToPoolTag.IsValid();
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
		
		NotifyID = context.getNotifyID(NotifyTag);
		
		for (auto& dataStore : *context.dataStores)
		{
			EntityDataRequestNotifyID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestNotifyTag));
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataRequestBackToPoolID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestBackToPoolTag));
		}
	}
	
	void ExecuteNotify(const uint16 indexDataID, const uint16 indexMember, FFluxPrimeArchetypeDataStore& dataStore)
	{
		FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
		FFluxPrimeWrapperInt32* entityDataRequestNotify = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataRequestNotifyID[indexDataID]));
		FFluxPrimeWrapperBool* entityDataRequestBackToPool = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataRequestBackToPoolID[indexDataID]));
		
		if (entityDataRequestNotify[indexMember].ValueInt32 != NotifyID) return;
		
		entityDataRequestBackToPool[indexMember].ValueBool = true;
		entityDataLocation[indexMember] = FVector::DownVector * 1000.0f;
		entityDataRequestNotify[indexMember].ValueInt32 = INDEX_NONE;
		UE_LOG(LogTemp, Log, TEXT("NOFITY:: DEAD"));
	}
};
