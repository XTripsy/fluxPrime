#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Framework/Core/FluxPrimeDataStore.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_DebugReadLocation, "FluxPrime.Tests.DebugRead.Location");

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeDataStoreReadTest, "FluxPrime.Debug.DataStoreRead",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeDataStoreReadTest::RunTest(const FString& Parameters)
{
	FFluxPrimeArchetypeDataStore Store;
	if (!TestTrue(TEXT("Buffer registers"),
		Store.AddBuffer(TAG_DebugReadLocation, TBaseStructure<FVector>::Get(),
			TBaseStructure<FVector>::Get()->GetStructureSize(), 4))) return false;

	Store.SetActiveEntityCount(3);
	FVector* Writable = reinterpret_cast<FVector*>(Store.GetRawBufferData(Store.GetDataID(TAG_DebugReadLocation)));
	Writable[1] = FVector(10.0f, 20.0f, 30.0f);

	const FFluxPrimeArchetypeDataStore& ConstStore = Store;
	int16 DataID = INDEX_NONE;
	TestTrue(TEXT("Const store resolves a matching type"),
		ConstStore.TryResolveTag(TAG_DebugReadLocation, TBaseStructure<FVector>::Get(), DataID));
	TestEqual(TEXT("Resolved id matches the writable lookup"),
		static_cast<int32>(DataID), Store.GetDataID(TAG_DebugReadLocation));

	int16 WrongID = INDEX_NONE;
	TestFalse(TEXT("Const store rejects a mismatched type"),
		ConstStore.TryResolveTag(TAG_DebugReadLocation, TBaseStructure<FRotator>::Get(), WrongID));

	const uint8* Peeked = ConstStore.PeekRawBufferData(DataID);
	TestTrue(TEXT("Const peek returns the same storage"), Peeked == reinterpret_cast<const uint8*>(Writable));
	TestTrue(TEXT("Const peek reads written data"),
		reinterpret_cast<const FVector*>(Peeked)[1].Equals(FVector(10.0f, 20.0f, 30.0f)));
	TestNull(TEXT("Const peek rejects an invalid id"), ConstStore.PeekRawBufferData(99));
	return true;
}
#endif
