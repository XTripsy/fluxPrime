#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Domains/Crowds/Data/FluxPrimeCrowdsConfigData.h"
#include "Generics/Systems/Navigation/FluxPrimeBaseNavigationSystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeNavigationWaypointTest, "FluxPrime.High.NavigationWaypoints",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeNavigationWaypointTest::RunTest(const FString& Parameters)
{
	const int32 capacity = static_cast<int32>(FluxConfig::WaypointsCount);

	TestTrue(TEXT("The waypoint budget is positive"), capacity > 0);
	TestEqual(TEXT("A short path keeps its count"),ClampWaypointCount(3, capacity), 3);
	TestEqual(TEXT("A full path keeps its count"),ClampWaypointCount(capacity, capacity), capacity);
	TestEqual(TEXT("The reported 17-waypoint path clamps to the budget"),ClampWaypointCount(17, capacity), capacity);
	TestEqual(TEXT("An oversized path is clamped to the budget"),ClampWaypointCount(capacity + 12, capacity), capacity);
	TestEqual(TEXT("A very long path is clamped to the budget"),ClampWaypointCount(1000, capacity), capacity);
	TestEqual(TEXT("A negative result is clamped to zero"),ClampWaypointCount(-1, capacity), 0);
	TestEqual(TEXT("A zero budget copies nothing"),ClampWaypointCount(5, 0), 0);

	const FVector EntityExtent = ComputeNavigationProjectionExtent(100, 1.5f);
	TestTrue(TEXT("Projection extent scales by entity size"), EntityExtent.Equals(FVector(150.0f, 150.0f, 300.0f)));
	const FVector TinyEntityExtent = ComputeNavigationProjectionExtent(0, 1.5f);
	TestTrue(TEXT("Projection extent never collapses for a zero size"), TinyEntityExtent.Equals(FVector(1.5f, 1.5f, 3.0f)));
	const FVector NegativeSizeExtent = ComputeNavigationProjectionExtent(-40, 1.5f);
	TestTrue(TEXT("Projection extent clamps a negative size"), NegativeSizeExtent.Equals(FVector(1.5f, 1.5f, 3.0f)));
	const FVector ZeroMultiplierExtent = ComputeNavigationProjectionExtent(100, 0.0f);
	TestTrue(TEXT("Projection extent honors a zero multiplier"), ZeroMultiplierExtent.IsNearlyZero());

	TestEqual(TEXT("A waypoint row carries positions and nothing else"),
		static_cast<int32>(sizeof(FFluxPrimeNavigationWaypoint)),
		capacity * static_cast<int32>(sizeof(FVector)));

	const FVector VerifiedPosition(100.0f, 200.0f, 50.0f);
	TestTrue(TEXT("A barely moved agent skips the projection query"),
		IsWithinConstrainThreshold(VerifiedPosition, VerifiedPosition + FVector(0.0f, 9.0f, 0.0f), 100.0f));
	TestFalse(TEXT("A moved agent pays for the projection query"),
		IsWithinConstrainThreshold(VerifiedPosition, VerifiedPosition + FVector(0.0f, 11.0f, 0.0f), 100.0f));
	TestTrue(TEXT("A zero threshold still skips a stationary agent"),
		IsWithinConstrainThreshold(VerifiedPosition, VerifiedPosition, 0.0f));

	return true;
}
#endif
