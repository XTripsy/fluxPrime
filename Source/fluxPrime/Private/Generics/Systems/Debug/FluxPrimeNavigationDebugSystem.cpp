#include "Generics/Systems/Debug/FluxPrimeNavigationDebugSystem.h"
#include "Framework/Systems/Debug/FluxPrimeDebugSystemRegistry.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperBool.h"
#include "Generics/Data/FluxPrimeNavigationData.h"
FLUXPRIME_REGISTER_DEBUG_SYSTEM(FFluxPrimeNavigationDebugSystem)
void FFluxPrimeNavigationDebugSystem::Initialized(const FFluxPrimeDebugInitContext& Context)
{
#if !UE_BUILD_SHIPPING
	bIsDataValid = false;
	if (!Context.View) return;
	for (int32 A = 0; A < Context.View->NumArchetypes(); ++A)
	{
		FFluxPrimeDebugBufferView L, T;
		if (Context.View->TryBind(A, CurrentLocation, L) && L.As<FVector>(TBaseStructure<FVector>::Get()) && Context.View->TryBind(A, TargetLocation, T) && T.As<FVector>(TBaseStructure<FVector>::Get())) bIsDataValid = true;
	}
#endif
}
void FFluxPrimeNavigationDebugSystem::ExecuteDebug(const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) const
{
#if !UE_BUILD_SHIPPING
	if (!Context.View) return;
	for (const auto& Ref : Context.Candidates)
	{
		FFluxPrimeDebugBufferView L, T, W, P, C, R, Wait;
		if (!Context.View->TryBind(Ref.Archetype, CurrentLocation, L) || !Context.View->TryBind(Ref.Archetype, TargetLocation, T)) continue;
		const auto* Positions = L.As<FVector>(TBaseStructure<FVector>::Get());
		const auto* Targets = T.As<FVector>(TBaseStructure<FVector>::Get());
		if (!Positions || !Targets || Ref.Row < 0 || Ref.Row >= L.Count) continue;
		Context.View->TryBind(Ref.Archetype, CurrentWaypoint, W);
		Context.View->TryBind(Ref.Archetype, Waypoints, P);
		Context.View->TryBind(Ref.Archetype, CountWaypoints, C);
		Context.View->TryBind(Ref.Archetype, NeedReplan, R);
		Context.View->TryBind(Ref.Archetype, WaitTime, Wait);
		const auto* Current = W.As<FVector>(TBaseStructure<FVector>::Get());
		const auto* Paths = P.As<FFluxPrimeNavigationWaypoint>(FFluxPrimeNavigationWaypoint::StaticStruct());
		const auto* Counts = C.As<FFluxPrimeWrapperUint8>(FFluxPrimeWrapperUint8::StaticStruct());
		const auto* Replans = R.As<FFluxPrimeWrapperBool>(FFluxPrimeWrapperBool::StaticStruct());
		const auto* Waits = Wait.As<FFluxPrimeWrapperFloat>(FFluxPrimeWrapperFloat::StaticStruct());
		const bool Replan = Replans && Replans[Ref.Row].ValueBool;
		const FColor Color = Replan ? ReplanColor : PathColor;
		FFluxPrimeDebugShape Line;
		Line.Start = Positions[Ref.Row]; Line.End = Current ? Current[Ref.Row] : Targets[Ref.Row]; Line.Color = Color; Line.Thickness = 2;
		FluxPrimeAddDebugShape(Frame, Line);
		FFluxPrimeDebugShape Point;
		Point.Kind = EFluxPrimeDebugShapeKind::Point; Point.Center = Targets[Ref.Row]; Point.Color = TargetColor; Point.Radius = 12;
		FluxPrimeAddDebugShape(Frame, Point);
		const int32 Count = Counts ? Counts[Ref.Row].ValueUint8 : 0;
		if (Paths)
		{
			const int32 Bound = FMath::Clamp(Count, 0, FMath::Clamp(MaxWaypointsDrawn, 0, int32(FluxConfig::WaypointsCount)));
			for (int32 I = 1; I < Bound; ++I) { Line.Start = Paths[Ref.Row].Waypoints[I-1]; Line.End = Paths[Ref.Row].Waypoints[I]; FluxPrimeAddDebugShape(Frame, Line); }
		}
		FString Text = FString::Printf(TEXT("Nav #%d | wp=%d | wait=%.1f"), Ref.Row, Count, Waits ? Waits[Ref.Row].ValueFloat : 0.f);
		if (Replan) Text += TEXT(" | replan");
		FluxPrimeAddDebugLabel(Frame, Ref.Archetype, Ref.Row, Ref.Location, Text, Color, FontScale);
	}
#endif
}
