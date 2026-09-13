#pragma once

#include "CoreMinimal.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsData.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsIdentity.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Core/FluxPrimeBaseModule.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperBool.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "FluxPrimeCrowdsSpawnerModule.generated.h"

USTRUCT(BlueprintType)
struct FSpawnerModuleDataTagConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataLocationTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataRotationTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataHealthTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataStateTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataMaxSpeedTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataConditionTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataRequestBackToPoolTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataRequestAbilityTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataRequestAnimationNotifyTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataStartTimeAnimationTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataPreviousAnimationFrameTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataTargetIdTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataLastOptimizeTimeTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataLastMoveTargetTimeTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataCountCorridorTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataRequestNeedReplanTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataCountWaypointsTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
    FGameplayTag EntityDataIdTag;

    bool IsValid() const
    {
       return EntityDataLocationTag.IsValid() 
          && EntityDataRotationTag.IsValid() 
          && EntityDataHealthTag.IsValid() 
          && EntityDataStateTag.IsValid() 
          && EntityDataMaxSpeedTag.IsValid() 
          && EntityDataConditionTag.IsValid() 
          && EntityDataRequestBackToPoolTag.IsValid() 
          && EntityDataRequestAbilityTag.IsValid() 
          && EntityDataRequestAnimationNotifyTag.IsValid() 
          && EntityDataStartTimeAnimationTag.IsValid() 
          && EntityDataPreviousAnimationFrameTag.IsValid() 
          && EntityDataTargetIdTag.IsValid() 
          && EntityDataLastOptimizeTimeTag.IsValid() 
          && EntityDataLastMoveTargetTimeTag.IsValid() 
          && EntityDataCountCorridorTag.IsValid() 
          && EntityDataRequestNeedReplanTag.IsValid() 
          && EntityDataCountWaypointsTag.IsValid()
          && EntityDataIdTag.IsValid();
    }
};

struct FFluxPrimeCrowdsSpawnerComponentContext
{
	FGetStateID getStateID = nullptr;
	
	TMap<FName, int8>* crowdsTypes = nullptr;
	
	UFluxPrimeCrowdsIdentity* identity = nullptr;
	FVector location; 
	FRotator rotation; 
	FGameplayTag newState;
	
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsSpawnerModule : public FFluxPrimeBaseModule
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FSpawnerModuleDataTagConfig DataTagConfig;

	TArray<int16> EntityDataLocationID, EntityDataRotationID, EntityDataHealthID, EntityDataStateID, EntityDataMaxSpeedID, EntityDataConditionID, EntityDataRequestBackToPoolID, EntityDataRequestAbilityID, EntityDataRequestAnimationNotifyID, EntityDataStartTimeAnimationID, EntityDataPreviousAnimationFrameID, EntityDataTargetIdID, EntityDataLastOptimizeTimeID, EntityDataLastMoveTargetTimeID, EntityDataCountCorridorID, EntityDataRequestNeedReplanID, EntityDataCountWaypointsID, EntityDataIdID;
	
public:
	void Initialized(const TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		for (auto& dataStore : dataStores)
		{
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataRotationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRotationTag));
			EntityDataHealthID.Add(dataStore.GetDataID(DataTagConfig.EntityDataHealthTag));
			EntityDataStateID.Add(dataStore.GetDataID(DataTagConfig.EntityDataStateTag));
			EntityDataMaxSpeedID.Add(dataStore.GetDataID(DataTagConfig.EntityDataMaxSpeedTag));
			EntityDataConditionID.Add(dataStore.GetDataID(DataTagConfig.EntityDataConditionTag));
			EntityDataRequestBackToPoolID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestBackToPoolTag));
			EntityDataRequestAbilityID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestAbilityTag));
			EntityDataRequestAnimationNotifyID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestAnimationNotifyTag));
			EntityDataStartTimeAnimationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataStartTimeAnimationTag));
			EntityDataPreviousAnimationFrameID.Add(dataStore.GetDataID(DataTagConfig.EntityDataPreviousAnimationFrameTag));
			EntityDataTargetIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataTargetIdTag));
			EntityDataLastOptimizeTimeID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLastOptimizeTimeTag));
			EntityDataLastMoveTargetTimeID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLastMoveTargetTimeTag));
			EntityDataCountCorridorID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCountCorridorTag));
			EntityDataRequestNeedReplanID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestNeedReplanTag));
			EntityDataCountWaypointsID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCountWaypointsTag));
			EntityDataIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataIdTag));
		}
	}
	
	void SpawnCrowd(FFluxPrimeCrowdsSpawnerComponentContext context)
	{
		auto& identity = *context.identity;
		auto& type = *context.crowdsTypes;
		if (!context.identity || !type.Contains(identity.Identity)) return;
		
		int16 indexDataID = *type.Find(identity.Identity); 
		auto& dataStore = (*context.dataStores)[indexDataID];
		
		if (dataStore.GetActiveEntityCount() >= dataStore.GetTotalEntityCount()) return;
		
		int8 health = identity.Health;
		int32 indexSelected = dataStore.GetActiveEntityCount();
		
		reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]))[indexSelected] = context.location;
		reinterpret_cast<FRotator*>(dataStore.GetRawBufferData(EntityDataRotationID[indexDataID]))[indexSelected] = FRotator(0, context.rotation.Yaw, 0);
		reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataHealthID[indexDataID]))[indexSelected].ValueInt16 = health;
		reinterpret_cast<FFluxPrimeWrapperInt8*>(dataStore.GetRawBufferData(EntityDataStateID[indexDataID]))[indexSelected].ValueInt8 = context.getStateID(context.newState);
		reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataMaxSpeedID[indexDataID]))[indexSelected].ValueFloat = identity.Speed;
		reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataConditionID[indexDataID]))[indexSelected].ValueBool = true;
		reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataRequestBackToPoolID[indexDataID]))[indexSelected].ValueBool = false;
		reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataRequestAbilityID[indexDataID]))[indexSelected].ValueBool = false;
		reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataRequestAnimationNotifyID[indexDataID]))[indexSelected].ValueInt32 = INDEX_NONE;
		reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataStartTimeAnimationID[indexDataID]))[indexSelected].ValueFloat = 0.0f;
		reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataPreviousAnimationFrameID[indexDataID]))[indexSelected].ValueFloat = 0.0f;
		reinterpret_cast<FFluxPrimeWrapperUint16*>(dataStore.GetRawBufferData(EntityDataTargetIdID[indexDataID]))[indexSelected].ValueUint16 = 0;
		reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataLastOptimizeTimeID[indexDataID]))[indexSelected].ValueFloat = 0.0f;
		reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataLastMoveTargetTimeID[indexDataID]))[indexSelected].ValueFloat = 0.0f;
		reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore.GetRawBufferData(EntityDataCountCorridorID[indexDataID]))[indexSelected].ValueUint8 = 0;
		reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataRequestNeedReplanID[indexDataID]))[indexSelected].ValueBool = true;
		reinterpret_cast<FFluxPrimeWrapperInt8*>(dataStore.GetRawBufferData(EntityDataCountWaypointsID[indexDataID]))[indexSelected].ValueInt8 = 0;
		
		int32 activeEntityCount = dataStore.GetActiveEntityCount();
		dataStore.SetActiveEntityCount(++activeEntityCount);
	}
};