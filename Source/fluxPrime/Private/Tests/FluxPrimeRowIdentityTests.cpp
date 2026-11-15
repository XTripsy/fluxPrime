#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RowIdentityValue, "FluxPrime.Tests.RowIdentity.Value");

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeRowIdentityTest, "FluxPrime.Medium.RowCompaction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeRowIdentityTest::RunTest(const FString& Parameters)
{
	FFluxPrimeArchetypeDataStore Store;
	Store.SetTotalEntityCount(3);
	Store.SetActiveEntityCount(3);
	TestTrue(TEXT("Row identity buffer registers"),
		Store.AddBuffer(TAG_RowIdentityValue, FFluxPrimeWrapperInt32::StaticStruct(),
			sizeof(FFluxPrimeWrapperInt32), 3));
	auto* Values = reinterpret_cast<FFluxPrimeWrapperInt32*>(
		Store.GetRawBufferData(Store.GetDataID(TAG_RowIdentityValue)));
	Values[0].ValueInt32 = 10;
	Values[1].ValueInt32 = 20;
	Values[2].ValueInt32 = 30;
	Store.SwapManualBuffer(1, 2);
	Store.SetActiveEntityCount(2);
	TestEqual(TEXT("Last entity moves into removed row"), Values[1].ValueInt32, 30);
	TestEqual(TEXT("Unrelated row is unchanged"), Values[0].ValueInt32, 10);
	TestEqual(TEXT("Removed entity lies outside active prefix"), Values[2].ValueInt32, 20);
	return true;
}
#endif
