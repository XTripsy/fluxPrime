#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Framework/Data/FluxPrimeDebugData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeDebugFrameTest, "FluxPrime.Debug.Frame",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeDebugFrameTest::RunTest(const FString& Parameters)
{
	FFluxPrimeDebugFrame Frame;
	Frame.Limits.MaxLabels = 2;
	Frame.Limits.MaxPrimitives = 1;
	FluxPrimeAddDebugLabel(Frame, 0, 5, FVector(100, 0, 0), TEXT("A"), FColor::White, 1.0f);
	FluxPrimeAddDebugLabel(Frame, 0, 5, FVector(100, 0, 0), TEXT("B"), FColor::White, 1.0f);
	FluxPrimeAddDebugLabel(Frame, 0, 5, FVector(100, 0, 0), TEXT("C"), FColor::White, 1.0f);
	TestEqual(TEXT("Label budget caps output"), Frame.Labels.Num(), 2);
	TestTrue(TEXT("Labels report truncation"), Frame.bLabelsTruncated);
	if (Frame.Labels.Num() == 2)
	{
		TestEqual(TEXT("Base height"), Frame.Labels[0].Location, FVector(100, 0, 180));
		TestEqual(TEXT("Stack height"), Frame.Labels[1].Location, FVector(100, 0, 204));
	}
	TestTrue(TEXT("First shape fits"), FluxPrimeAddDebugShape(Frame, FFluxPrimeDebugShape()));
	TestFalse(TEXT("Second shape rejected"), FluxPrimeAddDebugShape(Frame, FFluxPrimeDebugShape()));
	TestEqual(TEXT("Primitive cap"), Frame.Shapes.Num(), 1);
	TestTrue(TEXT("Primitives report truncation"), Frame.bPrimitivesTruncated);
	return true;
}
#endif
