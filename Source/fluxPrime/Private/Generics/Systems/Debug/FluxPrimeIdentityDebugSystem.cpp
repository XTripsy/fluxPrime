#include "Generics/Systems/Debug/FluxPrimeIdentityDebugSystem.h"
#include "Framework/Systems/Debug/FluxPrimeDebugSystemRegistry.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
FLUXPRIME_REGISTER_DEBUG_SYSTEM(FFluxPrimeIdentityDebugSystem)
void FFluxPrimeIdentityDebugSystem::Initialized(const FFluxPrimeDebugInitContext& Context)
{
#if !UE_BUILD_SHIPPING
	bIsDataValid = false;
	if (!Context.View) return;
	for (int32 A = 0; A < Context.View->NumArchetypes(); ++A)
	{
		FFluxPrimeDebugBufferView Buffer;
		if (Context.View->TryBind(A, Location, Buffer) && Buffer.As<FVector>(TBaseStructure<FVector>::Get())) bIsDataValid = true;
	}
#endif
}
void FFluxPrimeIdentityDebugSystem::ExecuteDebug(const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) const
{
#if !UE_BUILD_SHIPPING
	if (!Context.View) return;
	for (const auto& Ref : Context.Candidates)
	{
		FFluxPrimeDebugBufferView Buffer, Id;
		if (!Context.View->TryBind(Ref.Archetype, Location, Buffer) || !Buffer.As<FVector>(TBaseStructure<FVector>::Get()) || Ref.Row < 0 || Ref.Row >= Buffer.Count) continue;
		Context.View->TryBind(Ref.Archetype, StoredId, Id);
		const auto* Values = Id.As<FFluxPrimeWrapperInt16>(FFluxPrimeWrapperInt16::StaticStruct());
		const FString Name = Context.Archetypes.IsValidIndex(Ref.Archetype) ? Context.Archetypes[Ref.Archetype].Name.ToString() : FString::FromInt(Ref.Archetype);
		const FString Value = Values ? FString::FromInt(Values[Ref.Row].ValueInt16) : TEXT("n/a");
		FluxPrimeAddDebugLabel(Frame, Ref.Archetype, Ref.Row, Ref.Location, FString::Printf(TEXT("%s | renderRow=%d | storedId=%s"), *Name, Ref.Row, *Value), DebugColor, FontScale);
	}
#endif
}
