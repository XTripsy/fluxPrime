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

USTRUCT(BlueprintType)
struct FFluxPrimeStateLookUp
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FGameplayTag Identity;
	
	UPROPERTY(EditAnywhere)
	int8 CrowdsType;
	
	bool operator==(const FFluxPrimeStateLookUp& Other) const
	{
		return Identity == Other.Identity && CrowdsType == Other.CrowdsType;
	}
};

FORCEINLINE uint32 GetTypeHash(const FFluxPrimeStateLookUp& Value)
{
	return HashCombine(GetTypeHash(Value.Identity), GetTypeHash(Value.CrowdsType));
}