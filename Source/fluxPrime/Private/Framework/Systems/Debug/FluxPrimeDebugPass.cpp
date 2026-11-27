#include "Framework/Systems/Debug/FluxPrimeDebugPass.h"
#include "fluxPrime.h"

void FluxPrimeBuildDebugRuntime(const TArray<FInstancedStruct>& Systems, TArray<FFluxPrimeDebugRuntimeEntry>& Entries)
{
#if !UE_BUILD_SHIPPING
	Entries.Reset();
	for (int32 I = 0; I < Systems.Num(); ++I)
	{
		const auto* Base = Systems[I].GetPtr<FFluxPrimeBaseDebugSystem>();
		if (!Base || !Base->bEnabled) continue;
		const auto* Type = Systems[I].GetScriptStruct();
		const auto* Thunks = FFluxPrimeDebugSystemRegistry::Get().Find(Type);
		if (!Thunks)
		{
			static TSet<FName> Warned;
			if (!Warned.Contains(Type->GetFName()))
			{
				Warned.Add(Type->GetFName());
				UE_LOG(LogFluxPrime, Warning, TEXT("Unregistered debug system %s skipped"), *Type->GetName());
			}
			continue;
		}
		Entries.Add({I, Type, *Thunks, Entries.Num(), true});
	}
#endif
}
void FluxPrimeInitializeDebugSystems(const FFluxPrimeDebugReadView& View, TConstArrayView<FFluxPrimeDebugArchetypeMeta> Meta, TArray<FInstancedStruct>& Systems, TConstArrayView<FFluxPrimeDebugRuntimeEntry> Entries)
{
#if !UE_BUILD_SHIPPING
	for (const auto& Entry : Entries)
	{
		if (!Entry.bRuntimeEnabled || !Systems.IsValidIndex(Entry.SystemIndex) || !Entry.Thunks.Init) continue;
		auto* Base = Systems[Entry.SystemIndex].GetMutablePtr<FFluxPrimeBaseDebugSystem>();
		if (!Base) continue;
		Base->bIsDataValid = false;
		Base->VisualOrderIndex = Entry.VisualOrderIndex;
		Entry.Thunks.Init(Systems[Entry.SystemIndex], {&View, Meta});
		Entry.bInitialized = true;
		if (!Base->bIsDataValid)
		{
			static TSet<FName> Warned;
			const FName Name = Systems[Entry.SystemIndex].GetScriptStruct()->GetFName();
			if (!Warned.Contains(Name))
			{
				Warned.Add(Name);
				UE_LOG(LogFluxPrime, Warning, TEXT("Debug system %s has no valid archetype; check required tags, types and active buffer counts."), *Name.ToString());
			}
		}
	}
#endif
}
void FluxPrimeRunDebugPass(const FFluxPrimeDebugReadView& View, TConstArrayView<FFluxPrimeDebugArchetypeMeta> Meta, const FFluxPrimeDataConfig& Anchor, const FFluxPrimeDebugLimits& Limits, float BaseZ, float StepZ, const FVector& Camera, TArray<FInstancedStruct>& Systems, TConstArrayView<FFluxPrimeDebugRuntimeEntry> Entries, FFluxPrimeDebugFrame& Frame)
{
#if !UE_BUILD_SHIPPING
	Frame = FFluxPrimeDebugFrame();
	Frame.Limits = Limits;
	Frame.LabelBaseZ = BaseZ;
	Frame.LabelStepZ = StepZ;
	TArray<const FFluxPrimeDebugRuntimeEntry*> Active;
	for (const auto& Entry : Entries)
	{
		if (!Entry.bRuntimeEnabled || !Entry.Thunks.Execute || !Systems.IsValidIndex(Entry.SystemIndex)) continue;
		const auto* Base = Systems[Entry.SystemIndex].GetPtr<FFluxPrimeBaseDebugSystem>();
		if (Base && Base->bIsDataValid) Active.Add(&Entry);
	}
	if (Active.IsEmpty()) return;
	if (!Anchor.EntityDataTag.IsValid() || Anchor.ExpectedStructType != TBaseStructure<FVector>::Get())
	{
		static bool Warned = false;
		if (!Warned) UE_LOG(LogFluxPrime, Warning, TEXT("Debug CandidateLocation must bind FVector; no output."));
		Warned = true;
		return;
	}
	TArray<FFluxPrimeDebugRowRef> Candidates;
	bool bAnyAnchorBound = false;
	const double Distance = FMath::Max(0.0, static_cast<double>(Limits.MaxCameraDistance));
	for (int32 A = 0; A < View.NumArchetypes(); ++A)
	{
		FFluxPrimeDebugBufferView Buffer;
		if (!View.TryBind(A, Anchor, Buffer)) continue;
		const FVector* Positions = Buffer.As<FVector>(TBaseStructure<FVector>::Get());
		if (!Positions) continue;
		bAnyAnchorBound = true;
		for (int32 R = 0; R < Buffer.Count; ++R)
		{
			if (Frame.ScannedRows >= FMath::Max(0, Limits.MaxScannedRows)) { Frame.bScanTruncated = true; break; }
			++Frame.ScannedRows;
			const double D = FVector::DistSquared(Positions[R], Camera);
			if (FMath::IsFinite(D) && D <= Distance * Distance) Candidates.Add({A, R, D, Positions[R]});
		}
		if (Frame.bScanTruncated) break;
	}
	if (!bAnyAnchorBound && View.NumArchetypes() > 0)
	{
		static TSet<FGameplayTag> Warned;
		if (!Warned.Contains(Anchor.EntityDataTag))
		{
			Warned.Add(Anchor.EntityDataTag);
			UE_LOG(LogFluxPrime, Warning, TEXT("Debug CandidateLocation %s did not bind any archetype."), *Anchor.EntityDataTag.ToString());
		}
	}
	Candidates.Sort([](const auto& A, const auto& B) { if (A.DistanceSq != B.DistanceSq) return A.DistanceSq < B.DistanceSq; if (A.Archetype != B.Archetype) return A.Archetype < B.Archetype; return A.Row < B.Row; });
	// Candidate-major dispatch preserves global nearest-first priority across systems.
	for (const auto& Candidate : Candidates)
	{
		FFluxPrimeDebugExecuteContext Context{&View, Meta, MakeArrayView(&Candidate, 1), Camera, Candidates};
		for (const auto* Entry : Active)
		{
			if (Frame.Labels.Num() >= FMath::Max(0, Limits.MaxLabels) && Frame.Shapes.Num() >= FMath::Max(0, Limits.MaxPrimitives))
			{
				Frame.bLabelsTruncated = true;
				Frame.bPrimitivesTruncated = true;
				return;
			}
			Entry->Thunks.Execute(Systems[Entry->SystemIndex], Context, Frame);
		}
	}
#endif
}
