#include "Framework/Core/FluxPrimeDebugReadView.h"

bool FFluxPrimeDebugReadView::TryBind(const int32 ArchetypeIndex, const FFluxPrimeDataConfig& Config,
	FFluxPrimeDebugBufferView& OutView) const
{
	OutView = FFluxPrimeDebugBufferView();

	if (!Stores.IsValidIndex(ArchetypeIndex)) return false;

	const FFluxPrimeArchetypeDataStore& Store = Stores[ArchetypeIndex];
	int16 DataID = INDEX_NONE;
	if (!Store.TryResolveTag(Config.EntityDataTag, Config.ExpectedStructType, DataID)) return false;

	const int32 ActiveCount = Store.GetActiveEntityCount();
	if (ActiveCount < 0 || ActiveCount > Store.GetElementCount(DataID)) return false;

	const uint8* Data = Store.PeekRawBufferData(DataID);
	if (!Data) return false;

	OutView.Data = Data;
	OutView.Count = ActiveCount;
	OutView.Type = Config.ExpectedStructType;
	return true;
}
