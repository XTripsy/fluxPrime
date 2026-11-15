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
		if (!DataStoreIdentity)
		{
			IsAllDataValid = false;
			UE_LOG(LogFluxPrime, Error,
				TEXT("Data module cannot register buffers: DataStoreIdentity is not set."));
			return;
		}

		for (auto& pair : DataStoreIdentity->DataConfigEditors)
		{
			if (!hasAuthority && !pair.AvailableOnLocalClient) continue;
			
			if (!pair.StructType)
			{
				UE_LOG(LogFluxPrime, Error,
					TEXT("Data module rejected tag %s: no StructType is configured."),
					*pair.DataTag.ToString());
				IsAllDataValid = false;
				return;
			}

			const int32 structSize = pair.StructType->GetStructureSize();
			if (!dataStore.AddBuffer(pair.DataTag, pair.StructType, structSize, totalEntityCount))
			{
				UE_LOG(LogFluxPrime, Error,
					TEXT("Data module rejected tag %s: duplicate tag or invalid configuration. ")
					TEXT("Tag-to-buffer indices stay consistent because no buffer was allocated."),
					*pair.DataTag.ToString());
				IsAllDataValid = false;
				return;
			}
			
			UE_LOG(LogFluxPrime, VeryVerbose, TEXT("Allocated buffer for tag %s: %d bytes x %d entities"),
				*pair.DataTag.ToString(), structSize, totalEntityCount);
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