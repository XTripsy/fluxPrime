#include "Generics/Systems/Debug/FluxPrimeSpatialGridDebugSystem.h"
#include "Framework/Systems/Debug/FluxPrimeDebugSystemRegistry.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
FLUXPRIME_REGISTER_DEBUG_SYSTEM(FFluxPrimeSpatialGridDebugSystem)
void FFluxPrimeSpatialGridDebugSystem::Initialized(const FFluxPrimeDebugInitContext& Context)
{
#if !UE_BUILD_SHIPPING
	bIsDataValid = false;
	if (!Context.View) return;
	for (int32 A = 0; A < Context.View->NumArchetypes(); ++A)
	{
		FFluxPrimeDebugBufferView L, C;
		if (Context.View->TryBind(A, Location, L) && L.As<FVector>(TBaseStructure<FVector>::Get()) && Context.View->TryBind(A, CellId, C) && C.As<FFluxPrimeWrapperInt32>(FFluxPrimeWrapperInt32::StaticStruct())) bIsDataValid = true;
	}
#endif
}
void FFluxPrimeSpatialGridDebugSystem::ExecuteDebug(const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) const
{
#if !UE_BUILD_SHIPPING
	if (!Context.View) return;
	const auto Sample = Context.AllCandidates.IsEmpty() ? Context.Candidates : Context.AllCandidates;
	if (!Context.Candidates.IsEmpty() && !Sample.IsEmpty()
		&& Context.Candidates[0].Archetype == Sample[0].Archetype && Context.Candidates[0].Row == Sample[0].Row)
	{
		SampledOccupancy.Reset();
		FirstRows.Reset();
		for (const auto& Other : Sample)
		{
			FFluxPrimeDebugBufferView OL, OC;
			if (!Context.View->TryBind(Other.Archetype, Location, OL) || !OL.As<FVector>(TBaseStructure<FVector>::Get()) || !Context.View->TryBind(Other.Archetype, CellId, OC)) continue;
			const auto* Values = OC.As<FFluxPrimeWrapperInt32>(FFluxPrimeWrapperInt32::StaticStruct());
			if (!Values || Other.Row < 0 || Other.Row >= OC.Count) continue;
			const int32 Cell = Values[Other.Row].ValueInt32;
			++SampledOccupancy.FindOrAdd(Cell);
			if (!FirstRows.Contains(Cell)) FirstRows.Add(Cell, (uint64(uint32(Other.Archetype)) << 32) | uint32(Other.Row));
		}
	}
	for (const auto& Ref : Context.Candidates)
	{
		FFluxPrimeDebugBufferView L, C;
		if (!Context.View->TryBind(Ref.Archetype, Location, L) || !Context.View->TryBind(Ref.Archetype, CellId, C)) continue;
		const auto* Positions = L.As<FVector>(TBaseStructure<FVector>::Get());
		const auto* Cells = C.As<FFluxPrimeWrapperInt32>(FFluxPrimeWrapperInt32::StaticStruct());
		if (!Positions || !Cells || Ref.Row < 0 || Ref.Row >= L.Count) continue;
		const int32 Cell = Cells[Ref.Row].ValueInt32;
		const int32 Members = SampledOccupancy.FindRef(Cell);
		const uint64 Key = (uint64(uint32(Ref.Archetype)) << 32) | uint32(Ref.Row);
		const bool First = FirstRows.Contains(Cell) && FirstRows.FindRef(Cell) == Key;
		if (First && Cell >= 0 && CellWidth > 0 && CellHeight > 0 && int64(Cell) < int64(CellWidth) * CellHeight)
		{
			FFluxPrimeDebugShape Box;
			Box.Kind = EFluxPrimeDebugShapeKind::Box;
			Box.Center = FVector(Origin.X + (Cell % CellWidth + .5f) * CellSize, Origin.Y + (Cell / CellWidth + .5f) * CellSize, Positions[Ref.Row].Z);
			Box.Extent = FVector(CellSize * .5f, CellSize * .5f, BoxHeight * .5f);
			Box.Color = CellColor;
			FluxPrimeAddDebugShape(Frame, Box);
		}
		FluxPrimeAddDebugLabel(Frame, Ref.Archetype, Ref.Row, Ref.Location, FString::Printf(TEXT("Grid #%d | Stored cell (post-manager)=%d | members=%d (sampled)"), Ref.Row, Cell, Members), DebugColor, FontScale);
	}
#endif
}
