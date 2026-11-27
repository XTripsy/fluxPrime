#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Framework/Core/FluxPrimeDebugReadView.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_DebugViewLocation, "FluxPrime.Tests.DebugView.Location");

namespace
{
FFluxPrimeDataConfig DebugViewConfig(FGameplayTag Tag, UScriptStruct* Type)
{
	FFluxPrimeDataConfig Config;
	Config.EntityDataTag = Tag;
	Config.ExpectedStructType = Type;
	return Config;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeDebugReadViewTest, "FluxPrime.Debug.ReadView",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeDebugReadViewTest::RunTest(const FString& Parameters)
{
	TArray<FFluxPrimeArchetypeDataStore> Stores;
	Stores.SetNum(1);
	FFluxPrimeArchetypeDataStore& Store = Stores[0];
	if (!TestTrue(TEXT("Location buffer registers"),
		Store.AddBuffer(TAG_DebugViewLocation, TBaseStructure<FVector>::Get(),
			TBaseStructure<FVector>::Get()->GetStructureSize(), 3))) return false;
	Store.SetActiveEntityCount(2);

	reinterpret_cast<FVector*>(Store.GetRawBufferData(Store.GetDataID(TAG_DebugViewLocation)))[0] =
		FVector(1.0f, 2.0f, 3.0f);

	const FFluxPrimeDebugReadView View(Stores);
	TestEqual(TEXT("Archetype count is exposed"), View.NumArchetypes(), 1);
	TestEqual(TEXT("Active count is exposed"), View.GetActiveCount(0), 2);
	TestEqual(TEXT("Invalid archetype has no active count"), View.GetActiveCount(4), 0);

	FFluxPrimeDebugBufferView Bound;
	TestTrue(TEXT("Matching binding succeeds"),
		View.TryBind(0, DebugViewConfig(TAG_DebugViewLocation, TBaseStructure<FVector>::Get()), Bound));
	TestEqual(TEXT("Binding count uses the active prefix"), Bound.Count, 2);
	const FVector* Locations = Bound.As<FVector>(TBaseStructure<FVector>::Get());
	if (!TestNotNull(TEXT("Const element access returns data"), Locations)) return false;
	TestTrue(TEXT("Const element access reads the row"), Locations[0].Equals(FVector(1.0f, 2.0f, 3.0f)));
	TestNull(TEXT("Wrong element type is rejected"), Bound.As<FRotator>(TBaseStructure<FRotator>::Get()));
	TestEqual(TEXT("Binding does not change the active count"), Store.GetActiveEntityCount(), 2);
	TestTrue(TEXT("Binding does not change the row"), Locations[0].Equals(FVector(1.0f, 2.0f, 3.0f)));

	FFluxPrimeDebugBufferView WrongType;
	TestFalse(TEXT("Mismatched configured type fails"),
		View.TryBind(0, DebugViewConfig(TAG_DebugViewLocation, TBaseStructure<FRotator>::Get()), WrongType));
	TestNull(TEXT("Failed binding clears data"), WrongType.Data);

	Store.SetActiveEntityCount(4);
	FFluxPrimeDebugBufferView Oversized;
	TestFalse(TEXT("Active count beyond capacity fails"),
		View.TryBind(0, DebugViewConfig(TAG_DebugViewLocation, TBaseStructure<FVector>::Get()), Oversized));

	Store.SetActiveEntityCount(2);
	FFluxPrimeDebugBufferView Missing;
	TestFalse(TEXT("Unknown tag fails"),
		View.TryBind(0, DebugViewConfig(FGameplayTag(), TBaseStructure<FVector>::Get()), Missing));
	return true;
}
#endif
