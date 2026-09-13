#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsData.h"
#include "Framework/Core/FluxPrimeBaseDataModule.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "FluxPrimeCrowdsDataModule.generated.h"

struct FFluxPrimeCrowdsDataModuleContext
{
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* crowdsComponents = nullptr;
	TArray<FFluxPrimeArchetypeEntityCatalog>* entityCatalog = nullptr;
	
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT()
struct FFluxPrimeCrowdsDataModule : public FFluxPrimeBaseDataModule
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FGameplayTag IdDataTag;
	
	UPROPERTY(EditAnywhere)
	FGameplayTag SizeDataTag;
	
	TArray<int16> IdDataID, SizeDataID;
	
public:
	void NewInitialized(FFluxPrimeCrowdsDataModuleContext context)
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
			
			InitializedData(entityCatalog[i], dataStore);
		
			dataStore.InitializedLookUp(dataStore.GetTotalEntityCount());
		
			IdDataID.Add(dataStore.GetDataID(IdDataTag));
			SizeDataID.Add(dataStore.GetDataID(SizeDataTag));
			
			FFluxPrimeWrapperInt16* crowdsID = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(IdDataID[i]));
			FFluxPrimeWrapperInt16* crowdsSize = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(SizeDataID[i]));
			
			int16 size = entityCatalog[i].EntityIdentity->Size;
			
			for (int j = 0; j < entityCatalog[i].TotalEntityOnPool; ++j)
			{
				int32 id = crowdsComponents[i]->AddInstance(tempTransform, false);
				crowdsID[j].ValueInt16 = id;
				crowdsSize[j].ValueInt16 = size;
				
				dataStore.UpdateLookUpValue(id, j);
			}
			
			for (int k = 0; k < dataStore.GetTotalEntityCount(); ++k)
			{
				UE_LOG(LogTemp, Log, TEXT("INIT DATA TAG:: ID %d | TYPE %d"), crowdsID[k].ValueInt16, i);
			}
		}
	}
};