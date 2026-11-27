#include "Generics/Systems/Debug/FluxPrimeMovementDebugSystem.h"
#include "Framework/Systems/Debug/FluxPrimeDebugSystemRegistry.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
FLUXPRIME_REGISTER_DEBUG_SYSTEM(FFluxPrimeMovementDebugSystem)
namespace
{
bool BindMovementDebug(const FFluxPrimeDebugReadView& View, int32 A, const FFluxPrimeMovementDebugSystem& System, FFluxPrimeDebugBufferView* B)
{
	return View.TryBind(A, System.Location, B[0]) && View.TryBind(A, System.Velocity, B[1]) && View.TryBind(A, System.Waypoint, B[2]) && View.TryBind(A, System.MaxSpeed, B[3]) && B[0].As<FVector>(TBaseStructure<FVector>::Get()) && B[1].As<FVector>(TBaseStructure<FVector>::Get()) && B[2].As<FVector>(TBaseStructure<FVector>::Get()) && B[3].As<FFluxPrimeWrapperFloat>(FFluxPrimeWrapperFloat::StaticStruct());
}
}
void FFluxPrimeMovementDebugSystem::Initialized(const FFluxPrimeDebugInitContext& Context)
{
#if !UE_BUILD_SHIPPING
	bIsDataValid = false;
	if (!Context.View) return;
	for (int32 A = 0; A < Context.View->NumArchetypes(); ++A) { FFluxPrimeDebugBufferView B[4]; if (BindMovementDebug(*Context.View, A, *this, B)) bIsDataValid = true; }
#endif
}
void FFluxPrimeMovementDebugSystem::ExecuteDebug(const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) const
{
#if !UE_BUILD_SHIPPING
	if (!Context.View) return;
	for (const auto& Ref : Context.Candidates)
	{
		FFluxPrimeDebugBufferView B[4];
		if (!BindMovementDebug(*Context.View, Ref.Archetype, *this, B) || Ref.Row < 0 || Ref.Row >= B[0].Count) continue;
		const FString Name = Context.Archetypes.IsValidIndex(Ref.Archetype) ? Context.Archetypes[Ref.Archetype].Name.ToString() : FString::FromInt(Ref.Archetype);
		FluxPrimeAddDebugLabel(Frame, Ref.Archetype, Ref.Row, Ref.Location, FString::Printf(TEXT("%s #%d | v=%.0f | max=%.0f | wp=%.0f"), *Name, Ref.Row, B[1].As<FVector>(TBaseStructure<FVector>::Get())[Ref.Row].Size(), B[3].As<FFluxPrimeWrapperFloat>(FFluxPrimeWrapperFloat::StaticStruct())[Ref.Row].ValueFloat, FVector::Dist(B[0].As<FVector>(TBaseStructure<FVector>::Get())[Ref.Row], B[2].As<FVector>(TBaseStructure<FVector>::Get())[Ref.Row])), DebugColor, FontScale);
	}
#endif
}
