// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeEnum.generated.h"

UENUM(BlueprintType)
enum class EFluxPrimeCrowdAnimationNotify : uint8
{
	NotifyNone				UMETA(DisplayName = "NotifyNone"),
	NotifyAttack			UMETA(DisplayName = "NotifyAttack"),
	NotifySpawnVFX			UMETA(DisplayName = "NotifySpawnVFX"),
	NotifySpawnSFX			UMETA(DisplayName = "NotifySpawnSFX"),
	NotifyDead				UMETA(DisplayName = "NotifyDead"),
	Count UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EFluxPrimeCrowdState : uint8
{
	StateIdle				UMETA(DisplayName = "StateIdle"),
	StateWalk				UMETA(DisplayName = "StateWalk"),
	StateAbility			UMETA(DisplayName = "StateAbility"),
	StateDead				UMETA(DisplayName = "StateDead"),
	Count UMETA(Hidden)
};