#pragma once

#include "CoreMinimal.h"
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
	FFluxPrimeDataConfig EntityDataLocationConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRotationConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataCurrentWaypointLocationConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataHealthConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataStateConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataMaxSpeedConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataConditionConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRequestBackToPoolConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRequestAbilityConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRequestAnimationNotifyConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataStartTimeAnimationConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataPreviousAnimationFrameConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataTargetIdConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLastOptimizeTimeConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLastMoveTargetTimeConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataCountCorridorConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRequestNeedReplanConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataCountWaypointsConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataIdConfig;

	bool IsValid() const
	{
		return EntityDataLocationConfig.EntityDataTag.IsValid()
			&& EntityDataRotationConfig.EntityDataTag.IsValid()
			&& EntityDataCurrentWaypointLocationConfig.EntityDataTag.IsValid()
			&& EntityDataHealthConfig.EntityDataTag.IsValid()
			&& EntityDataStateConfig.EntityDataTag.IsValid()
			&& EntityDataMaxSpeedConfig.EntityDataTag.IsValid()
			&& EntityDataConditionConfig.EntityDataTag.IsValid()
			&& EntityDataRequestBackToPoolConfig.EntityDataTag.IsValid()
			&& EntityDataRequestAbilityConfig.EntityDataTag.IsValid()
			&& EntityDataRequestAnimationNotifyConfig.EntityDataTag.IsValid()
			&& EntityDataStartTimeAnimationConfig.EntityDataTag.IsValid()
			&& EntityDataPreviousAnimationFrameConfig.EntityDataTag.IsValid()
			&& EntityDataTargetIdConfig.EntityDataTag.IsValid()
			&& EntityDataLastOptimizeTimeConfig.EntityDataTag.IsValid()
			&& EntityDataLastMoveTargetTimeConfig.EntityDataTag.IsValid()
			&& EntityDataCountCorridorConfig.EntityDataTag.IsValid()
			&& EntityDataRequestNeedReplanConfig.EntityDataTag.IsValid()
			&& EntityDataCountWaypointsConfig.EntityDataTag.IsValid()
			&& EntityDataIdConfig.EntityDataTag.IsValid();
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

public:
	UPROPERTY(EditAnywhere)
	FSpawnerModuleDataTagConfig DataTagConfig;

private:
	UPROPERTY(EditAnywhere)
	float RandomOffsetPercent = 0.5f;
	bool HasAuthority = false;
	bool IsInitialized = false;
	TArray<int16> EntityDataLocationID, EntityDataRotationID, EntityDataCurrentWaypointLocationID, EntityDataHealthID, EntityDataStateID, EntityDataMaxSpeedID, EntityDataConditionID, EntityDataRequestBackToPoolID, EntityDataRequestAbilityID, EntityDataRequestAnimationNotifyID, EntityDataStartTimeAnimationID, EntityDataPreviousAnimationFrameID, EntityDataTargetIdID, EntityDataLastOptimizeTimeID, EntityDataLastMoveTargetTimeID, EntityDataCountCorridorID, EntityDataRequestNeedReplanID, EntityDataCountWaypointsID, EntityDataIdID;

public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores, const bool hasAuthority = true)
	{
		IsAllDataValid = true;
		IsInitialized = false;
		HasAuthority = hasAuthority;
		const int32 count = dataStores.Num();
		EntityDataLocationID.Init(INDEX_NONE, count);
		EntityDataRotationID.Init(INDEX_NONE, count);
		EntityDataCurrentWaypointLocationID.Init(INDEX_NONE, count);
		EntityDataHealthID.Init(INDEX_NONE, count);
		EntityDataStateID.Init(INDEX_NONE, count);
		EntityDataMaxSpeedID.Init(INDEX_NONE, count);
		EntityDataConditionID.Init(INDEX_NONE, count);
		EntityDataRequestBackToPoolID.Init(INDEX_NONE, count);
		EntityDataRequestAbilityID.Init(INDEX_NONE, count);
		EntityDataRequestAnimationNotifyID.Init(INDEX_NONE, count);
		EntityDataStartTimeAnimationID.Init(INDEX_NONE, count);
		EntityDataPreviousAnimationFrameID.Init(INDEX_NONE, count);
		EntityDataTargetIdID.Init(INDEX_NONE, count);
		EntityDataLastOptimizeTimeID.Init(INDEX_NONE, count);
		EntityDataLastMoveTargetTimeID.Init(INDEX_NONE, count);
		EntityDataCountCorridorID.Init(INDEX_NONE, count);
		EntityDataRequestNeedReplanID.Init(INDEX_NONE, count);
		EntityDataCountWaypointsID.Init(INDEX_NONE, count);
		EntityDataIdID.Init(INDEX_NONE, count);
		for (int32 i = 0; i < count; ++i)
		{
			auto& dataStore = dataStores[i];
			TryInitializedData(dataStore, DataTagConfig.EntityDataLocationConfig, EntityDataLocationID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRotationConfig, EntityDataRotationID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataCurrentWaypointLocationConfig, EntityDataCurrentWaypointLocationID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataStateConfig, EntityDataStateID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataConditionConfig, EntityDataConditionID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRequestBackToPoolConfig, EntityDataRequestBackToPoolID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRequestAbilityConfig, EntityDataRequestAbilityID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRequestAnimationNotifyConfig, EntityDataRequestAnimationNotifyID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataStartTimeAnimationConfig, EntityDataStartTimeAnimationID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataPreviousAnimationFrameConfig, EntityDataPreviousAnimationFrameID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataIdConfig, EntityDataIdID[i]);
			if (!HasAuthority) continue;
			TryInitializedData(dataStore, DataTagConfig.EntityDataHealthConfig, EntityDataHealthID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataMaxSpeedConfig, EntityDataMaxSpeedID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataTargetIdConfig, EntityDataTargetIdID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataLastOptimizeTimeConfig, EntityDataLastOptimizeTimeID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataLastMoveTargetTimeConfig, EntityDataLastMoveTargetTimeID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataCountCorridorConfig, EntityDataCountCorridorID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRequestNeedReplanConfig, EntityDataRequestNeedReplanID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataCountWaypointsConfig, EntityDataCountWaypointsID[i]);
		}
		IsInitialized = IsAllDataValid;
	}

private:
	FFluxPrimeArchetypeDataStore* ResolveSpawnTarget(const FFluxPrimeCrowdsSpawnerComponentContext& context, int32& outIndexDataID)
	{
		if (!context.identity)
		{
			UE_LOG(LogFluxPrime, Error, TEXT("Spawn rejected: the spawn request has no identity."));
			return nullptr;
		}
		if (!context.crowdsTypes || !context.dataStores || !context.getStateID)
		{
			UE_LOG(LogFluxPrime, Error, TEXT("Spawn rejected: archetype map, data stores or state lookup is unavailable."));
			return nullptr;
		}
		if (!IsInitialized || !IsAllDataValid) return nullptr;

		const int32 stateID = context.getStateID(context.newState);
		if (stateID == INDEX_NONE || stateID > TNumericLimits<int8>::Max())
		{
			UE_LOG(LogFluxPrime, Error,
				TEXT("Spawn rejected: state tag '%s' does not resolve to a valid global state id (got %d)."),
				*context.newState.ToString(), stateID);
			return nullptr;
		}

		const int8* foundIndex = context.crowdsTypes->Find(context.identity->Identity);
		if (!foundIndex || !context.dataStores->IsValidIndex(*foundIndex) || !EntityDataLocationID.IsValidIndex(*foundIndex))
		{
			UE_LOG(LogFluxPrime, Error, TEXT("Spawn rejected: identity '%s' has no initialized archetype."), *context.identity->Identity.ToString());
			return nullptr;
		}
		auto& dataStore = (*context.dataStores)[*foundIndex];
		if (dataStore.GetActiveEntityCount() >= dataStore.GetTotalEntityCount()) return nullptr;
		outIndexDataID = *foundIndex;
		return &dataStore;
	}

	void WriteSpawnFields(const FFluxPrimeCrowdsSpawnerComponentContext& context, FFluxPrimeArchetypeDataStore& dataStore,
		const int32 indexDataID, const int32 indexSelected)
	{
		reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]))[indexSelected] = context.location;
		reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataCurrentWaypointLocationID[indexDataID]))[indexSelected] = context.location;
		reinterpret_cast<FRotator*>(dataStore.GetRawBufferData(EntityDataRotationID[indexDataID]))[indexSelected] = FRotator(0, context.rotation.Yaw, 0);
		reinterpret_cast<FFluxPrimeWrapperInt8*>(dataStore.GetRawBufferData(EntityDataStateID[indexDataID]))[indexSelected].ValueInt8 = context.getStateID(context.newState);
		reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataConditionID[indexDataID]))[indexSelected].ValueBool = true;
		reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataRequestBackToPoolID[indexDataID]))[indexSelected].ValueBool = false;
		reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataRequestAbilityID[indexDataID]))[indexSelected].ValueBool = false;
		reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataRequestAnimationNotifyID[indexDataID]))[indexSelected].ValueInt32 = INDEX_NONE;
		reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataStartTimeAnimationID[indexDataID]))[indexSelected].ValueFloat = 0.0f;
		reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataPreviousAnimationFrameID[indexDataID]))[indexSelected].ValueFloat = 0.0f;
	}

public:
	void SpawnNetCrowd(FFluxPrimeCrowdsSpawnerComponentContext context)
	{
		int32 indexDataID = INDEX_NONE;
		auto* dataStore = ResolveSpawnTarget(context, indexDataID);
		if (!dataStore) return;
		const int32 indexSelected = dataStore->GetActiveEntityCount();
		WriteSpawnFields(context, *dataStore, indexDataID, indexSelected);
		dataStore->SetActiveEntityCount(indexSelected + 1);
	}

	void SpawnCrowd(FFluxPrimeCrowdsSpawnerComponentContext context)
	{
		int32 indexDataID = INDEX_NONE;
		auto* dataStore = ResolveSpawnTarget(context, indexDataID);
		if (!dataStore || !HasAuthority) return;
		const float baseSpeed = context.identity->Speed;
		const int16 health = context.identity->Health;
		const int32 indexSelected = dataStore->GetActiveEntityCount();
		WriteSpawnFields(context, *dataStore, indexDataID, indexSelected);
		reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore->GetRawBufferData(EntityDataHealthID[indexDataID]))[indexSelected].ValueInt16 = health;
		reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore->GetRawBufferData(EntityDataMaxSpeedID[indexDataID]))[indexSelected].ValueFloat = FMath::FRandRange(baseSpeed * (1.0f - RandomOffsetPercent), baseSpeed * (1.0f + RandomOffsetPercent));
		reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore->GetRawBufferData(EntityDataTargetIdID[indexDataID]))[indexSelected].ValueInt16 = 0;
		reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore->GetRawBufferData(EntityDataLastOptimizeTimeID[indexDataID]))[indexSelected].ValueFloat = 0.0f;
		reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore->GetRawBufferData(EntityDataLastMoveTargetTimeID[indexDataID]))[indexSelected].ValueFloat = 0.0f;
		reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore->GetRawBufferData(EntityDataCountCorridorID[indexDataID]))[indexSelected].ValueUint8 = 0;
		reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore->GetRawBufferData(EntityDataRequestNeedReplanID[indexDataID]))[indexSelected].ValueBool = true;
		reinterpret_cast<FFluxPrimeWrapperUint8*>(dataStore->GetRawBufferData(EntityDataCountWaypointsID[indexDataID]))[indexSelected].ValueUint8 = 0;
		dataStore->SetActiveEntityCount(indexSelected + 1);
	}
};
