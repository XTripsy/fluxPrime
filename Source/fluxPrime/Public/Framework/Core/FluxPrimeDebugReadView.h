#pragma once

#include "CoreMinimal.h"
#include "Containers/ArrayView.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/FluxPrimeDataStoreIdentity.h"

/** One validated const buffer view for a single debug inspection pass. */
struct FFluxPrimeDebugBufferView
{
	const uint8* Data = nullptr;
	int32 Count = 0;
	UScriptStruct* Type = nullptr;

	template <typename T>
	const T* As(const UScriptStruct* InType) const
	{
		return Type == InType ? reinterpret_cast<const T*>(Data) : nullptr;
	}
};

/**
 * Read-only view over a manager's archetype stores.
 *
 * Pointers and counts are valid for the pass that created the view. Rows are current active
 * prefix addresses, not persistent entity handles.
 */
struct FLUXPRIME_API FFluxPrimeDebugReadView
{
private:
	TConstArrayView<FFluxPrimeArchetypeDataStore> Stores;
	
public:
	FFluxPrimeDebugReadView() = default;

	explicit FFluxPrimeDebugReadView(TConstArrayView<FFluxPrimeArchetypeDataStore> InStores)
		: Stores(InStores)
	{
	}

	int32 NumArchetypes() const { return Stores.Num(); }

	int32 GetActiveCount(const int32 ArchetypeIndex) const
	{
		return Stores.IsValidIndex(ArchetypeIndex) ? Stores[ArchetypeIndex].GetActiveEntityCount() : 0;
	}

	bool TryBind(const int32 ArchetypeIndex, const FFluxPrimeDataConfig& Config,
		FFluxPrimeDebugBufferView& OutView) const;
};
