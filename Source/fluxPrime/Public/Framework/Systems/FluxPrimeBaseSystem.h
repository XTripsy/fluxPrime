#pragma once

#include "CoreMinimal.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "FluxPrimeBaseSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
	virtual ~FFluxPrimeBaseSystem() = default;
	
public:
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Is system can run on local server"))
	bool AvailableOnLocalServer;
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Is system can run on local client"))
	bool AvailableOnLocalClient;
};
