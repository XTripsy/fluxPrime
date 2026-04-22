// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeEnum.generated.h"

UENUM(BlueprintType)
enum class EFluxCrowdAnimationNotify : uint8
{
	NotifyNone				UMETA(DisplayName = "NotifyNone"),
	NotifyAttack			UMETA(DisplayName = "NotifyAttack"),
	NotifySpawnVFX			UMETA(DisplayName = "NotifySpawnVFX"),
	NotifySpawnSFX			UMETA(DisplayName = "NotifySpawnSFX")
};