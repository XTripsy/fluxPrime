#include "Generics/Systems/Debug/FluxPrimeBoidsDebugSystem.h"
#include "Framework/Systems/Debug/FluxPrimeDebugSystemRegistry.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
FLUXPRIME_REGISTER_DEBUG_SYSTEM(FFluxPrimeBoidsDebugSystem)
void FFluxPrimeBoidsDebugSystem::Initialized(const FFluxPrimeDebugInitContext& Context)
{
#if !UE_BUILD_SHIPPING
	bIsDataValid = false;
	if (!Context.View) return;
	for (int32 A = 0; A < Context.View->NumArchetypes(); ++A)
	{
		FFluxPrimeDebugBufferView L, S;
		if (Context.View->TryBind(A, Location, L) && L.As<FVector>(TBaseStructure<FVector>::Get()) && Context.View->TryBind(A, Size, S) && S.As<FFluxPrimeWrapperInt16>(FFluxPrimeWrapperInt16::StaticStruct())) bIsDataValid = true;
	}
#endif
}
void FFluxPrimeBoidsDebugSystem::ExecuteDebug(const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) const
{
#if !UE_BUILD_SHIPPING
	if (!Context.View) return;
	for (const auto& Ref : Context.Candidates)
	{
		FFluxPrimeDebugBufferView L, S;
		if (!Context.View->TryBind(Ref.Archetype, Location, L) || !Context.View->TryBind(Ref.Archetype, Size, S)) continue;
		const auto* Positions = L.As<FVector>(TBaseStructure<FVector>::Get());
		const auto* Sizes = S.As<FFluxPrimeWrapperInt16>(FFluxPrimeWrapperInt16::StaticStruct());
		if (!Positions || !Sizes || Ref.Row < 0 || Ref.Row >= L.Count) continue;
		const float Value = Sizes[Ref.Row].ValueInt16;
		FFluxPrimeDebugShape Circle;
		Circle.Kind = EFluxPrimeDebugShapeKind::Circle;
		Circle.Center = Positions[Ref.Row];
		Circle.Radius = Value * DiameterScale * .5f;
		Circle.Color = CircleColor;
		Circle.Thickness = 1.5f;
		FluxPrimeAddDebugShape(Frame, Circle);
		FluxPrimeAddDebugLabel(Frame, Ref.Archetype, Ref.Row, Ref.Location, FString::Printf(TEXT("Boids #%d | size=%.0f | radius=%.1f"), Ref.Row, Value, Circle.Radius), DebugColor, FontScale);
	}
#endif
}
