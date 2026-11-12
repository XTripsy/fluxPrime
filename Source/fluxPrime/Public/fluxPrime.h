// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

FLUXPRIME_API DECLARE_LOG_CATEGORY_EXTERN(LogFluxPrime, Log, All);

class FfluxPrimeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
