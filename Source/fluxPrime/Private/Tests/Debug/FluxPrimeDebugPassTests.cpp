#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Framework/Systems/Debug/FluxPrimeDebugPass.h"
#include "NativeGameplayTags.h"
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_DebugPassLocation, "FluxPrime.Tests.DebugPass.Location");
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeDebugPassTest, "FluxPrime.Debug.BudgetTruncation", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFluxPrimeDebugPassTest::RunTest(const FString& Parameters)
{
	TArray<FFluxPrimeArchetypeDataStore> Stores;
	Stores.SetNum(1);
	Stores[0].AddBuffer(TAG_DebugPassLocation, TBaseStructure<FVector>::Get(), sizeof(FVector), 1000);
	Stores[0].SetActiveEntityCount(1000);
	auto* Locations = reinterpret_cast<FVector*>(Stores[0].GetRawBufferData(0));
	for (int32 I = 0; I < 1000; ++I) Locations[I] = FVector(1000 - I, 0, 0);
	FFluxPrimeDataConfig Anchor;
	Anchor.EntityDataTag = TAG_DebugPassLocation;
	Anchor.ExpectedStructType = TBaseStructure<FVector>::Get();
	TArray<FInstancedStruct> Systems;
	Systems.Add(FInstancedStruct::Make<FFluxPrimeBaseDebugSystem>());
	TArray<FFluxPrimeDebugRuntimeEntry> Entries;
	FFluxPrimeDebugRuntimeEntry& Entry = Entries.AddDefaulted_GetRef();
	Entry.SystemIndex = 0;
	Entry.Thunks.Execute = [](const FInstancedStruct&, const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame)
	{
		for (const auto& Ref : Context.Candidates)
		{
			FluxPrimeAddDebugLabel(Frame, Ref.Archetype, Ref.Row, Ref.Location, FString::FromInt(Ref.Row), FColor::White, 1);
			FluxPrimeAddDebugShape(Frame, FFluxPrimeDebugShape());
		}
	};
	Systems[0].GetMutable<FFluxPrimeBaseDebugSystem>().bIsDataValid = true;
	FFluxPrimeDebugLimits Limits;
	Limits.MaxScannedRows = 1000;
	Limits.MaxLabels = 50;
	Limits.MaxPrimitives = 100;
	FFluxPrimeDebugFrame Frame;
	FluxPrimeRunDebugPass(FFluxPrimeDebugReadView(Stores), {}, Anchor, Limits, 100, 10, FVector::ZeroVector, Systems, Entries, Frame);
	TestEqual(TEXT("Label cap"), Frame.Labels.Num(), 50);
	TestEqual(TEXT("Shape cap independent of labels"), Frame.Shapes.Num(), 100);
	TestTrue(TEXT("Label truncation"), Frame.bLabelsTruncated);
	TestTrue(TEXT("Shape truncation"), Frame.bPrimitivesTruncated);
	if (Frame.Labels.Num()) TestEqual(TEXT("Nearest first"), Frame.Labels[0].Text, FString(TEXT("999")));
	Limits.MaxScannedRows = 10;
	FluxPrimeRunDebugPass(FFluxPrimeDebugReadView(Stores), {}, Anchor, Limits, 100, 10, FVector::ZeroVector, Systems, Entries, Frame);
	TestEqual(TEXT("Scan cap"), Frame.ScannedRows, 10);
	TestTrue(TEXT("Scan truncation"), Frame.bScanTruncated);
	Entries[0].bRuntimeEnabled = false;
	FluxPrimeRunDebugPass(FFluxPrimeDebugReadView(Stores), {}, Anchor, Limits, 100, 10, FVector::ZeroVector, Systems, Entries, Frame);
	TestEqual(TEXT("Disabled consumes no scan"), Frame.ScannedRows, 0);
	return true;
}
#endif
