#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeRowIdentityTest, "FluxPrime.Medium.RowCompaction",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeRowIdentityTest::RunTest(const FString& Parameters)
{
	FFluxPrimeArchetypeDataStore Store;
	auto* Values = static_cast<FFluxPrimeWrapperInt32*>(FMemory::Malloc(3 * sizeof(FFluxPrimeWrapperInt32)));
	for (int32 Index = 0; Index < 3; ++Index) new (&Values[Index]) FFluxPrimeWrapperInt32();
	Store.AddNewRawBufferData(reinterpret_cast<uint8*>(Values));
	Store.AddNewRawElementSizes(sizeof(FFluxPrimeWrapperInt32));
	Store.SetTotalEntityCount(3);
	Store.SetActiveEntityCount(3);
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
