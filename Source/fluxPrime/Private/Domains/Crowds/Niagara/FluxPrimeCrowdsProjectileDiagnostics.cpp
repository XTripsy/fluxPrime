#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileDiagnostics.h"
#include "ProfilingDebugging/CountersTrace.h"
#include "fluxPrime.h"
#include "HAL/IConsoleManager.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "UObject/UObjectIterator.h"

#include <atomic>

TRACE_DECLARE_ATOMIC_INT_COUNTER(FluxProjectileConsumed, TEXT("FluxPrime/Projectile/GraphConsumedRecords"));
TRACE_DECLARE_ATOMIC_INT_COUNTER(FluxProjectileBalls, TEXT("FluxPrime/Projectile/GraphBallSpawns"));
TRACE_DECLARE_ATOMIC_INT_COUNTER(FluxProjectileMuzzles, TEXT("FluxPrime/Projectile/GraphMuzzleBursts"));
TRACE_DECLARE_ATOMIC_INT_COUNTER(FluxProjectileGraphReports, TEXT("FluxPrime/Projectile/GraphReportCalls"));
namespace
{
	std::atomic<uint64> GraphConsumed{0}, GraphBalls{0}, GraphMuzzles{0};
}
void FFluxPrimeCrowdsProjectileDiagnostics::RecordGraphCounts(uint64 Consumed, uint64 Balls, uint64 Muzzles)
{
	GraphConsumed.fetch_add(Consumed, std::memory_order_relaxed);
	GraphBalls.fetch_add(Balls, std::memory_order_relaxed);
	GraphMuzzles.fetch_add(Muzzles, std::memory_order_relaxed);
	TRACE_COUNTER_ADD(FluxProjectileConsumed, Consumed);
	TRACE_COUNTER_ADD(FluxProjectileBalls, Balls);
	TRACE_COUNTER_ADD(FluxProjectileMuzzles, Muzzles);
	TRACE_COUNTER_INCREMENT(FluxProjectileGraphReports);
}
FFluxPrimeCrowdsProjectileDiagnostics::FGraphCounts FFluxPrimeCrowdsProjectileDiagnostics::GetGraphCounts()
{
	return {GraphConsumed.load(std::memory_order_relaxed), GraphBalls.load(std::memory_order_relaxed), GraphMuzzles.load(std::memory_order_relaxed)};
}
TRACE_DECLARE_INT_COUNTER(FluxProjectileNDCBytes, TEXT("FluxPrime/Projectile/EngineNDCAllocatedBytes_UNKNOWN"));
TRACE_DECLARE_INT_COUNTER(FluxProjectileCensus, TEXT("FluxPrime/Projectile/CensusRegisteredCannonballComponents"));
TRACE_DECLARE_INT_COUNTER(FluxMuzzleCensus, TEXT("FluxPrime/Projectile/CensusRegisteredMuzzleComponents"));

static FAutoConsoleCommand FluxProjectileCensusCommand(TEXT("fluxPrime.Projectile.Census"),
	TEXT("On-demand registered Cannonball/Muzzle component census across worlds; name-filtered, not authoritative ownership."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		int64 Projectile = 0, Muzzle = 0;
		for (TObjectIterator<UNiagaraComponent> It; It; ++It)
		{
			if (!IsValid(*It) || !It->IsRegistered() || !IsValid(It->GetAsset())) continue;
			const FString Name = It->GetAsset()->GetName();
			if (!Name.Contains(TEXT("Cannonball"))) continue;
			if (Name.Contains(TEXT("Muzzle"))) ++Muzzle; else ++Projectile;
		}
		TRACE_COUNTER_SET(FluxProjectileCensus, Projectile);
		TRACE_COUNTER_SET(FluxMuzzleCensus, Muzzle);
		UE_LOG(LogFluxPrime, Display, TEXT("Registered Cannonball components (all worlds, name filter): projectile=%lld muzzle=%lld"), Projectile, Muzzle);
	}));

TRACE_DECLARE_INT_COUNTER(FluxProjectileSubmitted, TEXT("FluxPrime/Projectile/Submitted"));
TRACE_DECLARE_INT_COUNTER(FluxProjectileAccepted, TEXT("FluxPrime/Projectile/Accepted"));
TRACE_DECLARE_INT_COUNTER(FluxProjectileFallback, TEXT("FluxPrime/Projectile/Fallback"));
TRACE_DECLARE_INT_COUNTER(FluxProjectileWrites, TEXT("FluxPrime/Projectile/NDCWriteAttempts"));
TRACE_DECLARE_INT_COUNTER(FluxProjectilePublished, TEXT("FluxPrime/Projectile/PublishedRecords"));
TRACE_DECLARE_INT_COUNTER(FluxProjectileSamples, TEXT("FluxPrime/Projectile/ExportedCollisionSamples"));
TRACE_DECLARE_INT_COUNTER(FluxProjectileDamage, TEXT("FluxPrime/Projectile/DamageApplications"));

#define FLUX_DECLARE_REJECTION(Name) TRACE_DECLARE_INT_COUNTER(FluxRejection##Name, TEXT("FluxPrime/Projectile/Fallback/" #Name));
FLUX_DECLARE_REJECTION(Disabled)
FLUX_DECLARE_REJECTION(InvalidWorld)
FLUX_DECLARE_REJECTION(MissingSubsystem)
FLUX_DECLARE_REJECTION(MissingSystem)
FLUX_DECLARE_REJECTION(MissingChannel)
FLUX_DECLARE_REJECTION(InvalidChannel)
FLUX_DECLARE_REJECTION(NonGlobalChannel)
FLUX_DECLARE_REJECTION(InvalidSchema)
FLUX_DECLARE_REJECTION(ConfigurationConflict)
FLUX_DECLARE_REJECTION(ComponentUnavailable)
FLUX_DECLARE_REJECTION(WriterUnavailable)
FLUX_DECLARE_REJECTION(Paused)
FLUX_DECLARE_REJECTION(CapacityExceeded)
#undef FLUX_DECLARE_REJECTION

namespace
{
	FFluxPrimeCrowdsProjectileDiagnostics Totals;
	const TCHAR* Names[] = {TEXT("None"), TEXT("Disabled"), TEXT("InvalidWorld"), TEXT("MissingSubsystem"),
		TEXT("MissingSystem"), TEXT("MissingChannel"), TEXT("InvalidChannel"), TEXT("NonGlobalChannel"),
		TEXT("InvalidSchema"), TEXT("ConfigurationConflict"), TEXT("ComponentUnavailable"), TEXT("WriterUnavailable"), TEXT("Paused"), TEXT("CapacityExceeded")};
}

const FFluxPrimeCrowdsProjectileDiagnostics& FFluxPrimeCrowdsProjectileDiagnostics::Get() { return Totals; }
const TCHAR* FFluxPrimeCrowdsProjectileDiagnostics::ReasonName(EFluxPrimeProjectileRejection Reason)
{
	return Names[static_cast<uint8>(Reason)];
}
void FFluxPrimeCrowdsProjectileDiagnostics::RecordResult(EFluxPrimeProjectileRejection Reason)
{
	check(IsInGameThread());
	if (Totals.Submitted == 0)
	{
		TRACE_COUNTER_SET(FluxProjectileNDCBytes, -1);
	}
	++Totals.Submitted;
	TRACE_COUNTER_INCREMENT(FluxProjectileSubmitted);
	if (Reason == EFluxPrimeProjectileRejection::None)
	{
		++Totals.Accepted;
		TRACE_COUNTER_INCREMENT(FluxProjectileAccepted);
		return;
	}
	const bool bTerminal = Reason == EFluxPrimeProjectileRejection::Paused || Reason == EFluxPrimeProjectileRejection::CapacityExceeded;
	if (!bTerminal)
	{
		++Totals.Fallback;
		TRACE_COUNTER_INCREMENT(FluxProjectileFallback);
	}
	const uint8 Index = static_cast<uint8>(Reason);
	++Totals.Rejections[Index];
#define FLUX_REJECTION_COUNTER(Name) case EFluxPrimeProjectileRejection::Name: { \
	TRACE_COUNTER_INCREMENT(FluxRejection##Name); break; }
	switch (Reason)
	{
		FLUX_REJECTION_COUNTER(Disabled)
		FLUX_REJECTION_COUNTER(InvalidWorld)
		FLUX_REJECTION_COUNTER(MissingSubsystem)
		FLUX_REJECTION_COUNTER(MissingSystem)
		FLUX_REJECTION_COUNTER(MissingChannel)
		FLUX_REJECTION_COUNTER(InvalidChannel)
		FLUX_REJECTION_COUNTER(NonGlobalChannel)
		FLUX_REJECTION_COUNTER(InvalidSchema)
		FLUX_REJECTION_COUNTER(ConfigurationConflict)
		FLUX_REJECTION_COUNTER(ComponentUnavailable)
		FLUX_REJECTION_COUNTER(WriterUnavailable)
		FLUX_REJECTION_COUNTER(Paused)
		FLUX_REJECTION_COUNTER(CapacityExceeded)
		default: break;
	}
#undef FLUX_REJECTION_COUNTER
	if (!bTerminal && Totals.Rejections[Index] == 1 && Reason != EFluxPrimeProjectileRejection::Disabled)
	{
		UE_LOG(LogFluxPrime, Display, TEXT("Projectile legacy fallback: %s (first occurrence; totals available in diagnostics)"), ReasonName(Reason));
	}
}
void FFluxPrimeCrowdsProjectileDiagnostics::RecordWrite(bool bPublished)
{
	if (bPublished) { ++Totals.Published; TRACE_COUNTER_INCREMENT(FluxProjectilePublished); }
	else { ++Totals.Writes; TRACE_COUNTER_INCREMENT(FluxProjectileWrites); }
}
void FFluxPrimeCrowdsProjectileDiagnostics::RecordSamples(int32 Count)
{
	Totals.ExportedSamples += Count;
	TRACE_COUNTER_ADD(FluxProjectileSamples, Count);
}
void FFluxPrimeCrowdsProjectileDiagnostics::RecordDamage()
{
	++Totals.DamageApplications;
	TRACE_COUNTER_INCREMENT(FluxProjectileDamage);
}
