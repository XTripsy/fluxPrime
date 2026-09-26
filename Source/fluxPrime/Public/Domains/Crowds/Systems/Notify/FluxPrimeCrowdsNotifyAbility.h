#pragma once

#include "CoreMinimal.h"
#include "Generics/Systems/Animation/FluxPrimeNotifyBase.h"
#include "FluxPrimeCrowdsNotifyAbility.generated.h"

USTRUCT(BlueprintType)
struct FNotifyAbilityDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRequestNotifyTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRequestAbilityTag;

	bool IsValid() const
	{
		return EntityDataRequestNotifyTag.IsValid() 
		   && EntityDataRequestAbilityTag.IsValid();
	}
};

USTRUCT()
struct FFluxPrimeCrowdsNotifyAbilityContext
{
	GENERATED_BODY()
	
	FGetNotifyID getNotifyID = nullptr;
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
		
		NotifyID = context.getNotifyID(NotifyTag);
		
		for (auto& dataStore : *context.dataStores)
		{
			EntityDataRequestNotifyID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestNotifyTag));
			EntityDataRequestAbilityID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestAbilityTag));
		}
	}
	
	void ExecuteNotify(const uint16 indexDataID, const uint16 indexMember, FFluxPrimeArchetypeDataStore& dataStore)
	{
		FFluxPrimeWrapperInt32* entityDataRequestNotify = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataRequestNotifyID[indexDataID]));
		FFluxPrimeWrapperBool* entityDataRequestAbility = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataRequestAbilityID[indexDataID]));
		
		if (entityDataRequestNotify[indexMember].ValueInt32 != NotifyID) return;
		
		entityDataRequestAbility[indexMember].ValueBool = true;
		entityDataRequestNotify[indexMember].ValueInt32 = INDEX_NONE;
		UE_LOG(LogTemp, Log, TEXT("NOTIFY:: ATTACK"));
	}
};
