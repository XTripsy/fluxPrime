#include "Generics/Systems/Debug/FluxPrimeAnimationDebugSystem.h"
#include "Framework/Systems/Debug/FluxPrimeDebugSystemRegistry.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
FLUXPRIME_REGISTER_DEBUG_SYSTEM(FFluxPrimeAnimationDebugSystem)
void FFluxPrimeAnimationDebugSystem::Initialized(const FFluxPrimeDebugInitContext& Context)
{
#if !UE_BUILD_SHIPPING
	bIsDataValid = false;
	if (!Context.View) return;
	for (int32 A = 0; A < Context.View->NumArchetypes(); ++A)
	{
		FFluxPrimeDebugBufferView L, S;
		if (Context.View->TryBind(A, Location, L) && L.As<FVector>(TBaseStructure<FVector>::Get()) && Context.View->TryBind(A, State, S) && S.As<FFluxPrimeWrapperInt8>(FFluxPrimeWrapperInt8::StaticStruct())) bIsDataValid = true;
	}
#endif
}
void FFluxPrimeAnimationDebugSystem::ExecuteDebug(const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) const
{
#if !UE_BUILD_SHIPPING
	if (!Context.View) return;
	for (const auto& Ref : Context.Candidates)
	{
		FFluxPrimeDebugBufferView L, S, A, T, N, Target;
		if (!Context.View->TryBind(Ref.Archetype, Location, L) || !Context.View->TryBind(Ref.Archetype, State, S)) continue;
		const auto* Positions = L.As<FVector>(TBaseStructure<FVector>::Get());
		const auto* States = S.As<FFluxPrimeWrapperInt8>(FFluxPrimeWrapperInt8::StaticStruct());
		if (!Positions || !States || Ref.Row < 0 || Ref.Row >= L.Count) continue;
		Context.View->TryBind(Ref.Archetype, AnimState, A);
		Context.View->TryBind(Ref.Archetype, StartTimeAnim, T);
		Context.View->TryBind(Ref.Archetype, RequestAnimationNotify, N);
		const auto* Anims = A.As<FFluxPrimeWrapperInt8>(FFluxPrimeWrapperInt8::StaticStruct());
		const auto* Times = T.As<FFluxPrimeWrapperFloat>(FFluxPrimeWrapperFloat::StaticStruct());
		const auto* Notifies = N.As<FFluxPrimeWrapperInt32>(FFluxPrimeWrapperInt32::StaticStruct());
		const FString AnimText = Anims ? FString::FromInt(Anims[Ref.Row].ValueInt8) : TEXT("n/a");
		const FString TimeText = Times ? FString::Printf(TEXT("%.2f"), Times[Ref.Row].ValueFloat) : TEXT("n/a");
		const FString NotifyText = Notifies ? FString::FromInt(Notifies[Ref.Row].ValueInt32) : TEXT("n/a");
		FluxPrimeAddDebugLabel(Frame, Ref.Archetype, Ref.Row, Ref.Location, FString::Printf(TEXT("Anim #%d | state=%d | anim=%s | t=%s | notify=%s"), Ref.Row, States[Ref.Row].ValueInt8, *AnimText, *TimeText, *NotifyText), DebugColor, FontScale);
		if (bDrawTargetLine && Context.View->TryBind(Ref.Archetype, TargetLocation, Target))
		{
			if (const auto* Targets = Target.As<FVector>(TBaseStructure<FVector>::Get()))
			{
				FFluxPrimeDebugShape Line;
				Line.Start = Positions[Ref.Row]; Line.End = Targets[Ref.Row]; Line.Color = TargetLineColor;
				FluxPrimeAddDebugShape(Frame, Line);
			}
		}
	}
#endif
}
