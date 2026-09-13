#pragma once

#include "CoreMinimal.h"
#include "Framework/Core/FluxPrimeBaseModule.h"
#include "Framework/Data/FluxPrimeEntityCatalogIdentity.h"
#include "Framework/Data/FluxPrimeDataStoreIdentity.h"
#include "Framework/Data/FluxPrimeIdentityData.h"
#include "FluxPrimeBaseDataModule.generated.h"

USTRUCT()
struct FFluxPrimeBaseDataModule : public FFluxPrimeBaseModule
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UFluxPrimeDataStoreIdentity> DataStoreIdentity;
	
private:
	void RegisterData(FFluxPrimeArchetypeDataStore& dataStore, const int32 totalEntityCount)
	{
		int32 index = 0;
		for (auto& pair : DataStoreIdentity->DataConfigEditors)
		{
			dataStore.AddNewDataID(pair.DataTag);
			
			int32 structSize = pair.StructType->GetStructureSize();
			uint8* allocatedMemory = static_cast<uint8*>(FMemory::Malloc(structSize * totalEntityCount));

			for (int32 i = 0; i < totalEntityCount; ++i) {
				pair.StructType->InitializeStruct(allocatedMemory + (i * structSize));
			}
			
			dataStore.AddNewRawBufferData(allocatedMemory);
			dataStore.AddNewRawElementSizes(structSize);
			
			UE_LOG(LogTemp, Log, TEXT("DATA :: TAG %s | ALLOCATE %p | INDEX %d"), *pair.DataTag.ToString(), allocatedMemory, index);
			index++;
		}
	}
	
protected:
	void InitializedData(FFluxPrimeArchetypeEntityCatalog& entityCatalog, FFluxPrimeArchetypeDataStore& dataStore)
	{
		int32 totalEntityCount = entityCatalog.TotalEntityOnPool;
		dataStore.SetTotalEntityCount(totalEntityCount);
		
		RegisterData(dataStore, totalEntityCount);
	}
};