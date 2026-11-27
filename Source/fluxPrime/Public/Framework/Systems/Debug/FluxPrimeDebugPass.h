#pragma once
#include "Framework/Systems/Debug/FluxPrimeDebugSystemRegistry.h"

struct FFluxPrimeDebugRuntimeEntry
{
	int32 SystemIndex = INDEX_NONE;
	const UScriptStruct* Type = nullptr;
	FFluxPrimeDebugThunks Thunks;
	int32 VisualOrderIndex = 0;
	bool bRuntimeEnabled = true;
	mutable bool bInitialized = false;
};
FLUXPRIME_API void FluxPrimeBuildDebugRuntime(const TArray<FInstancedStruct>& Systems, TArray<FFluxPrimeDebugRuntimeEntry>& Entries);
FLUXPRIME_API void FluxPrimeInitializeDebugSystems(const FFluxPrimeDebugReadView& View, TConstArrayView<FFluxPrimeDebugArchetypeMeta> Meta, TArray<FInstancedStruct>& Systems, TConstArrayView<FFluxPrimeDebugRuntimeEntry> Entries);
FLUXPRIME_API void FluxPrimeRunDebugPass(const FFluxPrimeDebugReadView& View, TConstArrayView<FFluxPrimeDebugArchetypeMeta> Meta, const FFluxPrimeDataConfig& Anchor, const FFluxPrimeDebugLimits& Limits, float BaseZ, float StepZ, const FVector& Camera, TArray<FInstancedStruct>& Systems, TConstArrayView<FFluxPrimeDebugRuntimeEntry> Entries, FFluxPrimeDebugFrame& Frame);
