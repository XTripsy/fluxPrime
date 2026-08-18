#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeStateBaseSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeStateBaseSystem
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
};
