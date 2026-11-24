#pragma once

#include "CoreMinimal.h"

// Values are stable trace categories. None means publication accepted, not consumer delivery.
enum class EFluxPrimeProjectileRejection : uint8
{
	None, Disabled, InvalidWorld, MissingSubsystem, MissingSystem, MissingChannel,
	InvalidChannel, NonGlobalChannel, InvalidSchema, ConfigurationConflict,
	ComponentUnavailable, WriterUnavailable, Paused, CapacityExceeded, Count
};

struct FLUXPRIME_API FFluxPrimeCrowdsProjectileDiagnostics
{
	uint64 Submitted = 0;
	uint64 Accepted = 0;
	uint64 Fallback = 0;
	uint64 Rejections[static_cast<uint8>(EFluxPrimeProjectileRejection::Count)] = {};
	uint64 Writes = 0;
	uint64 Published = 0;
	uint64 ExportedSamples = 0;
	uint64 DamageApplications = 0;

	struct FGraphCounts
	{
		uint64 Consumed = 0;
		uint64 Balls = 0;
		uint64 Muzzles = 0;
	};
	// Simulation-thread safe. Only graph execution may report these counts, never publication.
	static void RecordGraphCounts(uint64 Consumed, uint64 Balls, uint64 Muzzles);
	static FGraphCounts GetGraphCounts();

	static const FFluxPrimeCrowdsProjectileDiagnostics& Get();
	static void RecordResult(EFluxPrimeProjectileRejection Reason);
	static void RecordWrite(bool bPublished);
	static void RecordSamples(int32 Count);
	static void RecordDamage();
	static const TCHAR* ReasonName(EFluxPrimeProjectileRejection Reason);
};
