#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Generics/Systems/FluxPrimeMovementSystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeMovementAvoidanceTest, "FluxPrime.High.MovementAvoidance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeMovementAvoidanceTest::RunTest(const FString& Parameters)
{
	const FVector Desired(200.0f, 0.0f, 0.0f);
	
	const FVector Small = ClampAvoidanceVelocity(Desired, FVector(50.0f, 0.0f, 0.0f), 0.5f);
	TestTrue(TEXT("Avoidance below the cap is unchanged"), Small.Equals(FVector(50.0f, 0.0f, 0.0f)));

	const FVector Oversized = ClampAvoidanceVelocity(Desired, FVector(500.0f, 0.0f, 0.0f), 0.5f);
	TestTrue(TEXT("Oversized avoidance is clamped to half the desired speed"), FMath::IsNearlyEqual(Oversized.Size(), 100.0f));
	TestTrue(TEXT("Clamping preserves the avoidance direction"), Oversized.GetSafeNormal().Equals(FVector(1.0f, 0.0f, 0.0f)));

	const FVector NoDesired = ClampAvoidanceVelocity(FVector::ZeroVector, FVector(500.0f, 0.0f, 0.0f), 0.5f);
	TestTrue(TEXT("A stopped agent gets no avoidance"), NoDesired.IsNearlyZero());

	const FVector NoWeight = ClampAvoidanceVelocity(Desired, FVector(500.0f, 0.0f, 0.0f), 0.0f);
	TestTrue(TEXT("A zero weight disables avoidance"), NoWeight.IsNearlyZero());

	const FVector NegativeWeight = ClampAvoidanceVelocity(Desired, FVector(500.0f, 0.0f, 0.0f), -1.0f);
	TestTrue(TEXT("A negative weight disables avoidance"), NegativeWeight.IsNearlyZero());

	const FVector Diagonal = ClampAvoidanceVelocity(Desired, FVector(500.0f, 500.0f, 0.0f), 0.25f);
	TestTrue(TEXT("Diagonal avoidance keeps its cap"), FMath::IsNearlyEqual(Diagonal.Size(), 50.0f));
	TestTrue(TEXT("Diagonal avoidance keeps its direction"), Diagonal.GetSafeNormal().Equals(FVector(1.0f, 1.0f, 0.0f).GetSafeNormal()));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeMovementHoldClampTest, "FluxPrime.High.MovementHoldClamp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeMovementHoldClampTest::RunTest(const FString& Parameters)
{
	const FVector Avoidance(500.0f, 0.0f, 0.0f);

	const FVector Capped = ClampHoldAvoidanceVelocity(Avoidance, 200.0f, 0.5f);
	TestTrue(TEXT("Hold avoidance is capped by max speed and weight"), FMath::IsNearlyEqual(Capped.Size(), 100.0f));
	TestTrue(TEXT("Hold clamping preserves direction"), Capped.GetSafeNormal().Equals(FVector(1.0f, 0.0f, 0.0f)));

	const FVector Small = ClampHoldAvoidanceVelocity(FVector(40.0f, 0.0f, 0.0f), 200.0f, 0.5f);
	TestTrue(TEXT("Hold avoidance below the cap is unchanged"), Small.Equals(FVector(40.0f, 0.0f, 0.0f)));

	TestTrue(TEXT("Zero max speed holds a dead agent still"),
		ClampHoldAvoidanceVelocity(Avoidance, 0.0f, 0.5f).IsNearlyZero());
	TestTrue(TEXT("Zero weight disables hold avoidance"),
		ClampHoldAvoidanceVelocity(Avoidance, 200.0f, 0.0f).IsNearlyZero());
	TestTrue(TEXT("Negative weight disables hold avoidance"),
		ClampHoldAvoidanceVelocity(Avoidance, 200.0f, -1.0f).IsNearlyZero());

	const FVector Diagonal = ClampHoldAvoidanceVelocity(FVector(500.0f, 500.0f, 0.0f), 200.0f, 0.25f);
	TestTrue(TEXT("Diagonal hold avoidance keeps its cap"), FMath::IsNearlyEqual(Diagonal.Size(), 50.0f));
	TestTrue(TEXT("Diagonal hold avoidance keeps its direction"),
		Diagonal.GetSafeNormal().Equals(FVector(1.0f, 1.0f, 0.0f).GetSafeNormal()));

	return true;
}
#endif
