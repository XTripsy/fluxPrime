#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
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
		if (dataStore.TryResolveTag(dataTag, dataType, dataID)) return;

		IsAllDataValid = false;
		UE_LOG(LogFluxPrime, Error,
			TEXT("Data tag '%s' (expected struct '%s') was not found in this archetype's data store. ")
			TEXT("The owning system will not run. Add the tag and its struct to UFluxPrimeDataStoreIdentity."),
			*dataTag.ToString(),
			dataType ? *dataType->GetName() : TEXT("null"));
	}
	
public:
	virtual ~FFluxPrimeBaseSystem() = default;
};
