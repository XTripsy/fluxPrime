#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Generics/Data/FluxPrimeAnimationData.h"
#include "Generics/Systems/Animation/FluxPrimeNotifyBase.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperBool.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "FluxPrimeCrowdsNotifyAbility.generated.h"

USTRUCT(BlueprintType)
struct FNotifyAbilityDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRequestNotifyConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRequestAbilityConfig;

	bool IsValid() const
	{
		return EntityDataRequestNotifyConfig.EntityDataTag.IsValid() 
		   && EntityDataRequestAbilityConfig.EntityDataTag.IsValid();
	}
};

USTRUCT()
struct FFluxPrimeCrowdsNotifyAbilityContext
{
	GENERATED_BODY()
	
	FGetNotifyID getNotifyID;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsNotifyAbility : public FFluxPrimeNotifyBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FNotifyAbilityDataTagConfig DataTagConfig;

	TArray<int16> EntityDataRequestNotifyID, EntityDataRequestAbilityID;
	
public:
	void Initialized(FFluxPrimeCrowdsNotifyAbilityContext context)
	{
		check(context.getNotifyID);
		check(context.dataStores);
		
		int32 countData = context.dataStores->Num();
		EntityDataRequestNotifyID.Init(0, countData);
		EntityDataRequestAbilityID.Init(0, countData);
		
		NotifyID = context.getNotifyID(NotifyTag);
		
		int32 indexDataId = 0;
		for (auto& dataStore : *context.dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataRequestNotifyConfig, EntityDataRequestNotifyID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRequestAbilityConfig, EntityDataRequestAbilityID[indexDataId]);
			indexDataId++;
		}
	}
	
	void ExecuteNotify(const uint16 indexDataID, const uint16 indexMember, FFluxPrimeArchetypeDataStore& dataStore)
	{
		if (!IsAllDataValid)
		{
			UE_LOG(LogFluxPrime, Verbose, TEXT("NOTIFY ABILITY :: data invalid; system skipped"));
			return;
		}
		
		FFluxPrimeWrapperInt32* entityDataRequestNotify = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataRequestNotifyID[indexDataID]));
		FFluxPrimeWrapperBool* entityDataRequestAbility = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataRequestAbilityID[indexDataID]));
		
		check(entityDataRequestNotify);
		check(entityDataRequestAbility);
		
		if (entityDataRequestNotify[indexMember].ValueInt32 != NotifyID) return;
		
		entityDataRequestAbility[indexMember].ValueBool = true;
		entityDataRequestNotify[indexMember].ValueInt32 = INDEX_NONE;
	}
};
