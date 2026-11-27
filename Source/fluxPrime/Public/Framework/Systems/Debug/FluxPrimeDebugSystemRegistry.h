#pragma once
#include "Framework/Systems/Debug/FluxPrimeBaseDebugSystem.h"
#include "StructUtils/InstancedStruct.h"

using FFluxPrimeDebugInitFn = void(*)(FInstancedStruct&, const FFluxPrimeDebugInitContext&);
using FFluxPrimeDebugExecuteFn = void(*)(const FInstancedStruct&, const FFluxPrimeDebugExecuteContext&, FFluxPrimeDebugFrame&);
struct FFluxPrimeDebugThunks
{
	FFluxPrimeDebugInitFn Init = nullptr;
	FFluxPrimeDebugExecuteFn Execute = nullptr;
};

#if !UE_BUILD_SHIPPING
class FLUXPRIME_API FFluxPrimeDebugSystemRegistry
{
public:
	using FTypeGetter = UScriptStruct*(*)();
	static FFluxPrimeDebugSystemRegistry& Get();
	void Register(FTypeGetter Type, FFluxPrimeDebugInitFn Init, FFluxPrimeDebugExecuteFn Execute);
	const FFluxPrimeDebugThunks* Find(const UScriptStruct* Type) const;
	
private:
	struct FRegistration { FTypeGetter Type; FFluxPrimeDebugThunks Thunks; };
	TArray<FRegistration> Registrations;
};

template<typename T> struct TFluxPrimeDebugRegistrar
{
	TFluxPrimeDebugRegistrar()
	{
		// Resolve reflected types only after module initialization, not during static construction.
		FFluxPrimeDebugSystemRegistry::Get().Register(&T::StaticStruct,
			[](FInstancedStruct& Data, const FFluxPrimeDebugInitContext& Context) { Data.GetMutable<T>().Initialized(Context); },
			[](const FInstancedStruct& Data, const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) { Data.Get<T>().ExecuteDebug(Context, Frame); });
	}
};
#define FLUXPRIME_REGISTER_DEBUG_SYSTEM(TypeName) static TFluxPrimeDebugRegistrar<TypeName> GRegisterDebug##TypeName;
#else
#define FLUXPRIME_REGISTER_DEBUG_SYSTEM(TypeName)
#endif
