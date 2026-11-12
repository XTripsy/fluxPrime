#pragma once

#include "CoreMinimal.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/FluxPrimeDataStoreIdentity.h"
#include "FluxPrimeBaseSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Is system can run on local server", AllowPrivateAccess = true))
	bool AvailableOnLocalServer = true;
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Is system can run on local client", AllowPrivateAccess = true))
	bool AvailableOnLocalClient = false;
	
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
	
public:
	virtual ~FFluxPrimeBaseSystem() = default;
};
