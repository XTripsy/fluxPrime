#include "Framework/Systems/Debug/FluxPrimeDebugSystemRegistry.h"
#if !UE_BUILD_SHIPPING
FFluxPrimeDebugSystemRegistry& FFluxPrimeDebugSystemRegistry::Get()
{
	static FFluxPrimeDebugSystemRegistry Registry;
	return Registry;
}
void FFluxPrimeDebugSystemRegistry::Register(FTypeGetter Type, FFluxPrimeDebugInitFn Init, FFluxPrimeDebugExecuteFn Execute)
{
	Registrations.Add({Type, {Init, Execute}});
}
const FFluxPrimeDebugThunks* FFluxPrimeDebugSystemRegistry::Find(const UScriptStruct* Type) const
{
	for (const FRegistration& Registration : Registrations)
		if (Registration.Type() == Type) return &Registration.Thunks;
	return nullptr;
}
#endif
