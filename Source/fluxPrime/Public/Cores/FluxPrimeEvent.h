// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "FluxPrimeEvent.generated.h"

DECLARE_DELEGATE_OneParam(FOnCrowdsManagerActionChange, FInstancedStruct);
DECLARE_DELEGATE_OneParam(FOnCrowdsNetManagerActionChange, FInstancedStruct);

USTRUCT()
struct FFluxPrimeEvent
{
	GENERATED_BODY()
	
};
