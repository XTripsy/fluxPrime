#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Framework/Core/FluxPrimeBaseModule.h"
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
	void RegisterData(FFluxPrimeArchetypeDataStore& dataStore, const int32 totalEntityCount, const bool hasAuthority)
	{
		int32 index = 0;
		UScriptStruct* structType = nullptr;
		
		for (auto& pair : DataStoreIdentity->DataConfigEditors)
		{
			if (!hasAuthority && !pair.AvailableOnLocalClient) continue;
			
			dataStore.AddNewDataID(pair.DataTag);
			structType = pair.StructType;
			
			int32 structSize = structType->GetStructureSize();
			uint8* allocatedMemory = static_cast<uint8*>(FMemory::Malloc(structSize * totalEntityCount));

			for (int32 i = 0; i < totalEntityCount; ++i)
			{
				pair.StructType->InitializeStruct(allocatedMemory + (i * structSize));
			}
			
			dataStore.AddNewRawStructTypeData(structType);
			dataStore.AddNewRawBufferData(allocatedMemory);
			dataStore.AddNewRawElementSizes(structSize);
			
			UE_LOG(LogFluxPrime, VeryVerbose, TEXT("Allocated buffer for tag %s: %p, index %d, %d bytes x %d entities"),
				*pair.DataTag.ToString(), allocatedMemory, index, structSize, totalEntityCount);
			index++;
		}
	}
	
protected:
	void InitializedData(FFluxPrimeArchetypeEntityCatalog& entityCatalog, FFluxPrimeArchetypeDataStore& dataStore, const bool hasAuthority)
	{
		int32 totalEntityCount = entityCatalog.TotalEntityOnPool;
		dataStore.SetTotalEntityCount(totalEntityCount);
		
		RegisterData(dataStore, totalEntityCount, hasAuthority);
	}
};