#pragma once

#include "CoreMinimal.h"
#include "Framework/Data/FluxPrimeDataStoreIdentity.h"
#include "FluxPrimeNotifyBase.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeNotifyBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	FGameplayTag NotifyTag;
	
	int32 NotifyID = INDEX_NONE;
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
};
