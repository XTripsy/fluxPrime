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
	FGameplayTag EntityDataLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRotationTag;

	UPROPERTY(EditAnywhere)
	FGameplayTag EntityDataIdTag;
	
	UPROPERTY(EditAnywhere)
	FGameplayTag EntityDataSizeTag;
	
	UPROPERTY(EditAnywhere)
	FGameplayTag EntityDataDamageTag;

	bool IsValid() const
	{
		return EntityDataLocationTag.IsValid() 
		   && EntityDataRotationTag.IsValid() 
		   && EntityDataIdTag.IsValid()
		   && EntityDataSizeTag.IsValid()
		   && EntityDataDamageTag.IsValid();
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
	
	TArray<int16> EntityDataLocationID, EntityDataRotationID, EntityDataIdID, EntityDataSizeID, EntityDataDamageID;
	
public:
	void Initialized(FFluxPrimeCrowdsDataModuleContext context)
	{
		check(context.crowdsComponents);
		check(context.entityCatalog);
		check(context.dataStores);
		
		auto& dataStores = *context.dataStores;
		auto& crowdsComponents =  *context.crowdsComponents;
		auto& entityCatalog =  *context.entityCatalog;
		
		dataStores.Init(FFluxPrimeArchetypeDataStore(), entityCatalog.Num());
		
		FTransform tempTransform;
		tempTransform.SetLocation(FVector::DownVector * 1000.0f);
		
		for (int i = 0; i < entityCatalog.Num(); ++i)
		{
			auto& dataStore = dataStores[i];
			
			InitializedData(entityCatalog[i], dataStore, context.hasAuthority);
		
			dataStore.InitializedLookUp(dataStore.GetTotalEntityCount());
		
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataRotationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRotationTag));
			EntityDataIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataIdTag));
			EntityDataSizeID.Add(dataStore.GetDataID(DataTagConfig.EntityDataSizeTag));
			EntityDataDamageID.Add(dataStore.GetDataID(DataTagConfig.EntityDataDamageTag));
			
			FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[i]));
			FRotator* entityDataRotation = reinterpret_cast<FRotator*>(dataStore.GetRawBufferData(EntityDataRotationID[i]));
			FFluxPrimeWrapperInt16* entityDataID = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataIdID[i]));
			FFluxPrimeWrapperInt16* entityDataSize = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataSizeID[i]));
			FFluxPrimeWrapperFloat* entityDataDamage = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataDamageID[i]));
			
			check(entityDataLocation);
			check(entityDataRotation);
			check(entityDataID);
			check(entityDataSize);
			
			int16 size = entityCatalog[i].EntityIdentity->Size;
			float damage = Cast<UFluxPrimeCrowdsIdentity>(entityCatalog[i].EntityIdentity)->Damage;
			
			for (int j = 0; j < entityCatalog[i].TotalEntityOnPool; ++j)
			{
				int32 id = crowdsComponents[i]->AddInstance(tempTransform, false);
				entityDataLocation[i] = FVector::ZeroVector;
				entityDataRotation[i] = FRotator::ZeroRotator;
				entityDataID[j].ValueInt16 = id;
				entityDataSize[j].ValueInt16 = size;
				entityDataDamage[j].ValueFloat = damage;
				
				dataStore.UpdateLookUpValue(id, j);
			}
		}
	}
};