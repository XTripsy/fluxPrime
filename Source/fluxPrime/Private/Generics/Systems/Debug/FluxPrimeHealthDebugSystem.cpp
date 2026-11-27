#include "Generics/Systems/Debug/FluxPrimeHealthDebugSystem.h"
#include "Framework/Systems/Debug/FluxPrimeDebugSystemRegistry.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperBool.h"
FLUXPRIME_REGISTER_DEBUG_SYSTEM(FFluxPrimeHealthDebugSystem)
void FFluxPrimeHealthDebugSystem::Initialized(const FFluxPrimeDebugInitContext& Context)
{
#if !UE_BUILD_SHIPPING
	bIsDataValid = false;
	if (!Context.View) return;
	for (int32 A = 0; A < Context.View->NumArchetypes(); ++A)
	{
		FFluxPrimeDebugBufferView L, H;
		if (Context.View->TryBind(A, Location, L) && L.As<FVector>(TBaseStructure<FVector>::Get()) && Context.View->TryBind(A, Health, H) && H.As<FFluxPrimeWrapperInt16>(FFluxPrimeWrapperInt16::StaticStruct())) bIsDataValid = true;
	}
#endif
}
void FFluxPrimeHealthDebugSystem::ExecuteDebug(const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) const
{
#if !UE_BUILD_SHIPPING
	if (!Context.View) return;
	for (const auto& Ref : Context.Candidates)
	{
		FFluxPrimeDebugBufferView L, H, A;
		if (!Context.View->TryBind(Ref.Archetype, Location, L) || !Context.View->TryBind(Ref.Archetype, Health, H)) continue;
		const auto* Positions = L.As<FVector>(TBaseStructure<FVector>::Get());
		const auto* Values = H.As<FFluxPrimeWrapperInt16>(FFluxPrimeWrapperInt16::StaticStruct());
		if (!Positions || !Values || Ref.Row < 0 || Ref.Row >= L.Count) continue;
		Context.View->TryBind(Ref.Archetype, ConditionAlive, A);
		const auto* Alive = A.As<FFluxPrimeWrapperBool>(FFluxPrimeWrapperBool::StaticStruct());
		const int32 Hp = Values[Ref.Row].ValueInt16;
		const bool IsAlive = Alive ? Alive[Ref.Row].ValueBool : Hp > 0;
		const auto Color = IsAlive ? AliveColor : DeadColor;
		const int32 MaxHealth = Context.Archetypes.IsValidIndex(Ref.Archetype) ? Context.Archetypes[Ref.Archetype].MaxHealth : 0;
		if (bDrawBar && MaxHealth > 0)
		{
			FFluxPrimeDebugShape Bar;
			Bar.Kind = EFluxPrimeDebugShapeKind::Box;
			Bar.Center = Positions[Ref.Row] + FVector(0,0,BarZ);
			Bar.Extent = FVector(FMath::Max(0.f, BarWidth) * .5f, FMath::Max(0.f, BarHeight) * .5f, .5f);
			Bar.Color = FColor::Black;
			FluxPrimeAddDebugShape(Frame, Bar);
			const float Ratio = FMath::Clamp(float(Hp) / MaxHealth, 0.f, 1.f);
			if (Ratio > 0)
			{
				Bar.Center.X += -Bar.Extent.X + Bar.Extent.X * Ratio;
				Bar.Extent.X *= Ratio;
				Bar.Color = Color;
				FluxPrimeAddDebugShape(Frame, Bar);
			}
		}
		FluxPrimeAddDebugLabel(Frame, Ref.Archetype, Ref.Row, Ref.Location, FString::Printf(TEXT("Health #%d | hp=%d | %s"), Ref.Row, Hp, IsAlive ? TEXT("alive") : TEXT("dead")), Color, FontScale);
	}
#endif
}
