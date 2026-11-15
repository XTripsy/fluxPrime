#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_DataStoreTestLocation, "FluxPrime.Tests.DataStore.Location");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_DataStoreTestSize, "FluxPrime.Tests.DataStore.Size");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_DataStoreTestState, "FluxPrime.Tests.DataStore.State");

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeDataStoreRegistrationTest, "FluxPrime.High.DataStoreRegistration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeDataStoreRegistrationTest::RunTest(const FString& Parameters)
{
	FFluxPrimeArchetypeDataStore store;

	TestTrue(TEXT("First valid tag registers"),
		store.AddBuffer(TAG_DataStoreTestLocation, FFluxPrimeWrapperInt16::StaticStruct(), sizeof(FFluxPrimeWrapperInt16), 4));
	TestTrue(TEXT("Second valid tag registers"),
		store.AddBuffer(TAG_DataStoreTestSize, FFluxPrimeWrapperInt16::StaticStruct(), sizeof(FFluxPrimeWrapperInt16), 4));
	TestTrue(TEXT("Third valid tag registers"),
		store.AddBuffer(TAG_DataStoreTestState, FFluxPrimeWrapperInt8::StaticStruct(), sizeof(FFluxPrimeWrapperInt8), 4));

	TestEqual(TEXT("Tags keep their registration order"), store.GetDataID(TAG_DataStoreTestLocation), 0);
	TestEqual(TEXT("Second tag follows the first"), store.GetDataID(TAG_DataStoreTestSize), 1);
	TestEqual(TEXT("Third tag follows the second"), store.GetDataID(TAG_DataStoreTestState), 2);

	TestFalse(TEXT("Duplicate tag is rejected"),
		store.AddBuffer(TAG_DataStoreTestLocation, FFluxPrimeWrapperInt16::StaticStruct(), sizeof(FFluxPrimeWrapperInt16), 4));
	TestEqual(TEXT("Rejected duplicate leaves the mapping unchanged"), store.GetDataID(TAG_DataStoreTestLocation), 0);
	TestEqual(TEXT("Rejected duplicate does not shift later tags"), store.GetDataID(TAG_DataStoreTestState), 2);

	uint8* locationBuffer = store.GetRawBufferData(store.GetDataID(TAG_DataStoreTestLocation));
	TestNotNull(TEXT("Registered buffer is readable"), locationBuffer);
	reinterpret_cast<FFluxPrimeWrapperInt16*>(locationBuffer)[3].ValueInt16 = 42;
	TestEqual(TEXT("Buffer capacity covers every row"),
		reinterpret_cast<FFluxPrimeWrapperInt16*>(locationBuffer)[3].ValueInt16, 42);

	TestFalse(TEXT("Invalid struct type is rejected"),
		store.AddBuffer(TAG_DataStoreTestState, nullptr, sizeof(FFluxPrimeWrapperInt16), 4));

	return true;
}
#endif
