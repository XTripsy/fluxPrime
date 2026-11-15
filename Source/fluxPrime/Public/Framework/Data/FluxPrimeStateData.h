#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "FluxPrimeStateData.generated.h"

using FGetStateID = TFunction<int32(FGameplayTag)>;

/** Runtime state ids travel in int8 buffers and transition fields, so the global table cannot exceed this. */
constexpr int32 FluxPrimeMaxGlobalStateCount = TNumericLimits<int8>::Max() + 1;

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