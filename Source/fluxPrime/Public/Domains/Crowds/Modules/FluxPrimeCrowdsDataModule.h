#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Core/FluxPrimeBaseDataModule.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsIdentity.h"
#include "FluxPrimeCrowdsDataModule.generated.h"

USTRUCT(BlueprintType)
struct FDataModuleDataTagConfig
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLocationConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRotationConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataIdConfig;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataSizeConfig;

	bool IsValid() const
	{
		return EntityDataLocationConfig.EntityDataTag.IsValid()
			&& EntityDataRotationConfig.EntityDataTag.IsValid()
			&& EntityDataIdConfig.EntityDataTag.IsValid()
			&& EntityDataSizeConfig.EntityDataTag.IsValid();
	}
};

struct FFluxPrimeCrowdsDataModuleContext
{
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* crowdsComponents = nullptr;
	TArray<FFluxPrimeArchetypeEntityCatalog>* entityCatalog = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
	bool hasAuthority = false;
};

USTRUCT()
struct FFluxPrimeCrowdsDataModule : public FFluxPrimeBaseDataModule
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	FDataModuleDataTagConfig DataTagConfig;
	TArray<int16> EntityDataLocationID, EntityDataRotationID, EntityDataIdID, EntityDataSizeID;

	void Initialized(FFluxPrimeCrowdsDataModuleContext context)
	{
		IsAllDataValid = true;
		EntityDataLocationID.Reset();
		EntityDataRotationID.Reset();
		EntityDataIdID.Reset();
		EntityDataSizeID.Reset();
		if (!context.crowdsComponents || !context.entityCatalog || !context.dataStores ||
			context.crowdsComponents->Num() != context.entityCatalog->Num())
		{
			IsAllDataValid = false;
			UE_LOG(LogFluxPrime, Error, TEXT("Data module cannot initialize: invalid context or component count."));
			return;
		}
		auto& dataStores = *context.dataStores;
		auto& crowdsComponents = *context.crowdsComponents;
		auto& entityCatalog = *context.entityCatalog;
		const int32 count = entityCatalog.Num();
		EntityDataLocationID.Init(INDEX_NONE, count);
		EntityDataRotationID.Init(INDEX_NONE, count);
		EntityDataIdID.Init(INDEX_NONE, count);
		EntityDataSizeID.Init(INDEX_NONE, count);
		for (int32 i = 0; i < count; ++i)
		{
			if (!entityCatalog[i].EntityIdentity || !crowdsComponents[i] || entityCatalog[i].TotalEntityOnPool <= 0)
			{
				IsAllDataValid = false;
				UE_LOG(LogFluxPrime, Error, TEXT("Data module cannot initialize archetype %d: invalid identity, component or pool size."), i);
				return;
			}
		}
		// Build separately so failed validation leaves the existing pool untouched.
		TArray<FFluxPrimeArchetypeDataStore> pendingStores;
		pendingStores.SetNum(count);
		for (int32 i = 0; i < count; ++i)
		{
			auto& dataStore = pendingStores[i];
			InitializedData(entityCatalog[i], dataStore, context.hasAuthority);
			TryInitializedData(dataStore, DataTagConfig.EntityDataLocationConfig, EntityDataLocationID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRotationConfig, EntityDataRotationID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataIdConfig, EntityDataIdID[i]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataSizeConfig, EntityDataSizeID[i]);
		}
		if (!IsAllDataValid) return;
		dataStores = MoveTemp(pendingStores);
		FTransform tempTransform;
		tempTransform.SetLocation(FVector::DownVector * 1000.0f);
		for (int32 i = 0; i < count; ++i)
		{
			auto& dataStore = dataStores[i];
			auto* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[i]));
			auto* entityDataRotation = reinterpret_cast<FRotator*>(dataStore.GetRawBufferData(EntityDataRotationID[i]));
			auto* entityDataID = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataIdID[i]));
			auto* entityDataSize = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataSizeID[i]));
			crowdsComponents[i]->ClearInstances();
			for (int32 j = 0; j < entityCatalog[i].TotalEntityOnPool; ++j)
			{
				entityDataLocation[j] = FVector::ZeroVector;
				entityDataRotation[j] = FRotator::ZeroRotator;
				entityDataID[j].ValueInt16 = crowdsComponents[i]->AddInstance(tempTransform, false);
				entityDataSize[j].ValueInt16 = entityCatalog[i].EntityIdentity->Size;
			}
		}
	}
};
