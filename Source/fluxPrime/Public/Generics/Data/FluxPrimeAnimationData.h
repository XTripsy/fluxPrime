#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsConfigData.h"
#include "FluxPrimeAnimationData.generated.h"

using FGetNotifyID = TFunction<int32(FGameplayTag)>;

USTRUCT(BlueprintType)
struct FFluxCrowdsAnimationNotify
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FGameplayTag AnimationNotifyTag;
	
	UPROPERTY(EditAnywhere)
	int32 AnimationNotifyFrame = 0;
};

USTRUCT(BlueprintType)
struct FFluxCrowdsRuntimeAnimationNotify
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	int32 AnimationNotifyID;
	
	UPROPERTY(EditAnywhere)
	int32 AnimationNotifyFrame = 0;
};

USTRUCT(BlueprintType)
struct FFluxPrimeRuntimeAnimationNotifyData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FFluxCrowdsRuntimeAnimationNotify AnimationNotify[FluxConfig::AnimationArrayCount];
	
	UPROPERTY()
	int8 AnimationNotifyCount;
};

USTRUCT(BlueprintType)
struct FFluxPrimeRuntimeAnimationNotify
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TArray<FFluxPrimeRuntimeAnimationNotifyData> RuntimeAnimationNotifyData;
};

USTRUCT(BlueprintType)
struct FFluxPrimeRuntimeAnimationMapping
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TArray<uint32> AnimationStart;
	
	UPROPERTY(EditAnywhere)
	TArray<uint32> AnimationEnd;
	
	UPROPERTY(EditAnywhere)
	TArray<bool> AnimationForceNotify;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsAnimationMapping
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FFluxCrowdsAnimationNotify AnimationNotify[FluxConfig::AnimationArrayCount];
	
	UPROPERTY(EditAnywhere)
	uint32 AnimationStart;
	
	UPROPERTY(EditAnywhere)
	uint32 AnimationEnd;
	
	UPROPERTY(EditAnywhere)
	bool AnimationLoop;
};

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationDefinition
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FGameplayTag StateTag;
	
	UPROPERTY(EditAnywhere)
	int8 AnimationIndex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 AnimationStartFrame;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 AnimationEndFrame;
	
	UPROPERTY(EditAnywhere)
	bool AnimationForceNotify = false;
	
	UPROPERTY(EditAnywhere)
	FFluxCrowdsAnimationNotify AnimationNotifies[FluxConfig::AnimationArrayCount];
};