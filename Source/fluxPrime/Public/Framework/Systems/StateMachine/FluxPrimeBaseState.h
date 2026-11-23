#pragma once

#include "CoreMinimal.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/FluxPrimeDataStoreIdentity.h"
#include "FluxPrimeBaseState.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeBaseState
{
	GENERATED_BODY()
	
protected:
	bool IsAllDataValid = true;
	
protected:
	void TryInitializedData(FFluxPrimeArchetypeDataStore& dataStore, FFluxPrimeDataConfig dataConfig, int16& dataID)
	{
		dataID = INDEX_NONE;
		FGameplayTag dataTag = dataConfig.EntityDataTag;
		UScriptStruct* dataType = dataConfig.ExpectedStructType;
		
		if (!IsAllDataValid) return;
		IsAllDataValid = dataStore.TryResolveTag(dataTag, dataType, dataID);
	}

	/** Resolves data a state can run without. A missing or mistyped input leaves the ID unset and keeps the state valid. */
	void TryResolvedOptionalData(FFluxPrimeArchetypeDataStore& dataStore, const FFluxPrimeDataConfig& dataConfig, int16& dataID)
	{
		dataID = INDEX_NONE;
		if (!dataConfig.EntityDataTag.IsValid() || dataConfig.ExpectedStructType == nullptr) return;
		dataStore.TryResolveTag(dataConfig.EntityDataTag, dataConfig.ExpectedStructType, dataID);
	}
	
	/* turunan struct ini perlu implement function dengan nama yang sama, untuk dapat mengeksekusi logic statenya
	void ExecuteState(float deltaTime)
	{
		
	}*/
};
