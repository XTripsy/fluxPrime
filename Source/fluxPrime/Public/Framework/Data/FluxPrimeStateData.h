#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "FluxPrimeStateData.generated.h"

using FGetStateID = TFunction<int32(FGameplayTag)>;

USTRUCT()
struct FFluxPrimeStateDataEditor
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FGameplayTag IdentityState;
	
	UPROPERTY(EditAnywhere, meta=(BaseStruct="FluxPrimeBaseState", ExcludeBaseStruct))
	FInstancedStruct State;
	
	UPROPERTY(EditAnywhere, meta=(BaseStruct="FluxPrimeBaseStateTransition", ExcludeBaseStruct))
	FInstancedStruct Transition;
};