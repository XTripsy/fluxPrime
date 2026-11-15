#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Generics/Systems/FluxPrimeGroundHeightSystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeGroundHeightSelectionTest, "FluxPrime.High.GroundHeightSelection",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace
{
bool NearlyEqual(const float A, const float B)
{
	return FMath::IsNearlyEqual(A, B, 0.01f);
}
}

bool FFluxPrimeGroundHeightSelectionTest::RunTest(const FString& Parameters)
{
	const float Offset = 10.0f;
	const float Step = 100.0f;

	TestTrue(TEXT("No baked floor keeps the current height"),
		NearlyEqual(SelectGroundHeightTarget(0.0f, nullptr, 0, Step, Offset), 0.0f));

	const float OnFloor[] = {0.0f};
	TestTrue(TEXT("A floor at the current height applies the offset"),
		NearlyEqual( SelectGroundHeightTarget(0.0f, OnFloor, 1, Step, Offset), 10.0f));

	const float LowerFloor[] = {-40.0f};
	TestTrue(TEXT("A floor below within step height is used"),
		NearlyEqual( SelectGroundHeightTarget(0.0f, LowerFloor, 1, Step, Offset), -30.0f));

	TestTrue(TEXT("An agent far above a floor falls to it"),
		NearlyEqual( SelectGroundHeightTarget(300.0f, OnFloor, 1, Step, Offset), 10.0f));

	const float StepUpFloor[] = {50.0f};
	TestTrue(TEXT("A floor above within step height is used"),
		NearlyEqual( SelectGroundHeightTarget(0.0f, StepUpFloor, 1, Step, Offset), 60.0f));

	const float CeilingOnly[] = {300.0f};
	TestTrue(TEXT("A floor beyond step height never pulls an agent upward"),
		NearlyEqual( SelectGroundHeightTarget(0.0f, CeilingOnly, 1, Step, Offset), 0.0f));

	const float GroundAndCeiling[] = {0.0f, 80.0f};
	TestTrue(TEXT("Ground under a low ceiling wins over the ceiling"),
		NearlyEqual( SelectGroundHeightTarget(0.0f, GroundAndCeiling, 2, Step, Offset), 10.0f));

	const float MultiFloor[] = {0.0f, 250.0f, 500.0f};
	TestTrue(TEXT("The highest floor below the agent is chosen"),
		NearlyEqual( SelectGroundHeightTarget(300.0f, MultiFloor, 3, Step, Offset), 260.0f));

	const float GroundAndLedge[] = {0.0f, 350.0f};
	TestTrue(TEXT("A far-below floor is preferred over an overhead ledge"),
		NearlyEqual( SelectGroundHeightTarget(300.0f, GroundAndLedge, 2, Step, Offset), 10.0f));

	const float StackedSteps[] = {50.0f, 80.0f};
	TestTrue(TEXT("The closest reachable step above is chosen"),
		NearlyEqual( SelectGroundHeightTarget(0.0f, StackedSteps, 2, Step, Offset), 60.0f));

	TestTrue(TEXT("A negative step height disables step-up"),
		NearlyEqual( SelectGroundHeightTarget(0.0f, StepUpFloor, 1, -1.0f, Offset), 0.0f));
	TestTrue(TEXT("A negative step height still falls to a lower floor"),
		NearlyEqual( SelectGroundHeightTarget(100.0f, OnFloor, 1, -1.0f, Offset), 10.0f));

	return true;
}
#endif
