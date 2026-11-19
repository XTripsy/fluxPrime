#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "NativeGameplayTags.h"
#include "Generics/Systems/FluxPrimeMovementSystem.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_MovementHoldLocation, "FluxPrime.Tests.MovementHold.Location");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_MovementHoldRotation, "FluxPrime.Tests.MovementHold.Rotation");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_MovementHoldAcceleration, "FluxPrime.Tests.MovementHold.Acceleration");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_MovementHoldVelocity, "FluxPrime.Tests.MovementHold.Velocity");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_MovementHoldWaypoint, "FluxPrime.Tests.MovementHold.Waypoint");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_MovementHoldMaxSpeed, "FluxPrime.Tests.MovementHold.MaxSpeed");

namespace
{
struct FMovementHoldFixture
{
	FFluxPrimeMeshOrientationConfig Orientation;
	TArray<FFluxPrimeArchetypeDataStore> Stores;
	FFluxPrimeMovementSystem Movement;

	FMovementHoldFixture()
	{
		Orientation.ForwardYawOffsetDegrees = 0.0f;
		Movement.MeshOrientation = &Orientation;

		Stores.SetNum(1);
		FFluxPrimeArchetypeDataStore& Store = Stores[0];
		Store.SetTotalEntityCount(5);
		Store.SetActiveEntityCount(5);

		auto Add = [&Store](FGameplayTag Tag, UScriptStruct* Type)
		{
			Store.AddBuffer(Tag, Type, Type->GetStructureSize(), 5);
		};
		Add(TAG_MovementHoldLocation, TBaseStructure<FVector>::Get());
		Add(TAG_MovementHoldRotation, TBaseStructure<FRotator>::Get());
		Add(TAG_MovementHoldAcceleration, TBaseStructure<FVector>::Get());
		Add(TAG_MovementHoldVelocity, TBaseStructure<FVector>::Get());
		Add(TAG_MovementHoldWaypoint, TBaseStructure<FVector>::Get());
		Add(TAG_MovementHoldMaxSpeed, FFluxPrimeWrapperFloat::StaticStruct());

		auto Config = [](FFluxPrimeDataConfig& Data, FGameplayTag Tag, UScriptStruct* Type)
		{
			Data.EntityDataTag = Tag;
			Data.ExpectedStructType = Type;
		};
		Config(Movement.DataTagConfig.EntityDataLocationConfig, TAG_MovementHoldLocation, TBaseStructure<FVector>::Get());
		Config(Movement.DataTagConfig.EntityDataRotationConfig, TAG_MovementHoldRotation, TBaseStructure<FRotator>::Get());
		Config(Movement.DataTagConfig.EntityDataAccelerationConfig, TAG_MovementHoldAcceleration, TBaseStructure<FVector>::Get());
		Config(Movement.DataTagConfig.EntityDataVelocityConfig, TAG_MovementHoldVelocity, TBaseStructure<FVector>::Get());
		Config(Movement.DataTagConfig.EntityDataCurrentWaypointConfig, TAG_MovementHoldWaypoint, TBaseStructure<FVector>::Get());
		Config(Movement.DataTagConfig.EntityDataMaxSpeedConfig, TAG_MovementHoldMaxSpeed, FFluxPrimeWrapperFloat::StaticStruct());

		Movement.Initialized(Stores);
	}

	template<typename T>
	T* Buffer(FGameplayTag Tag)
	{
		return reinterpret_cast<T*>(Stores[0].GetRawBufferData(Stores[0].GetDataID(Tag)));
	}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeMovementHoldTest, "FluxPrime.High.MovementHold",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeMovementHoldTest::RunTest(const FString& Parameters)
{
	FMovementHoldFixture Fixture;
	if (!TestTrue(TEXT("Movement system resolves every hold tag"), Fixture.Movement.IsAllDataValid)) return false;

	// Full steering response so the moving row's expected speed is exact.
	Fixture.Movement.SteeringResponsiveness = 1000.0f;

	FVector* Location = Fixture.Buffer<FVector>(TAG_MovementHoldLocation);
	FRotator* Rotation = Fixture.Buffer<FRotator>(TAG_MovementHoldRotation);
	FVector* Acceleration = Fixture.Buffer<FVector>(TAG_MovementHoldAcceleration);
	FVector* Velocity = Fixture.Buffer<FVector>(TAG_MovementHoldVelocity);
	FVector* Waypoint = Fixture.Buffer<FVector>(TAG_MovementHoldWaypoint);
	FFluxPrimeWrapperFloat* MaxSpeed = Fixture.Buffer<FFluxPrimeWrapperFloat>(TAG_MovementHoldMaxSpeed);

	// Rows 0/1 hold and overlap. Row 2 is dead (max speed 0). Rows 3/4 still steer.
	Location[0] = FVector(0.0f, 0.0f, 30.0f);
	Location[1] = FVector(20.0f, 0.0f, 0.0f);
	Location[2] = FVector(100.0f, 0.0f, 0.0f);
	Location[3] = FVector(500.0f, 0.0f, 0.0f);
	Location[4] = FVector(800.0f, 0.0f, 0.0f);

	Rotation[0] = FRotator(0.0f, 45.0f, 0.0f);
	Rotation[1] = FRotator::ZeroRotator;
	Rotation[2] = FRotator::ZeroRotator;
	Rotation[3] = FRotator::ZeroRotator;
	Rotation[4] = FRotator(0.0f, 90.0f, 0.0f);

	Acceleration[0] = FVector(500.0f, 0.0f, 500.0f);
	Acceleration[1] = FVector(-50.0f, 0.0f, 0.0f);
	Acceleration[2] = FVector(500.0f, 0.0f, 0.0f);
	Acceleration[3] = FVector(100.0f, 0.0f, 0.0f);
	Acceleration[4] = FVector(500.0f, 0.0f, 0.0f);

	Velocity[0] = FVector(200.0f, 0.0f, 0.0f);
	Velocity[1] = FVector(0.0f, 200.0f, 0.0f);
	Velocity[2] = FVector(50.0f, 0.0f, 0.0f);
	Velocity[3] = FVector::ZeroVector;
	Velocity[4] = FVector::ZeroVector;

	MaxSpeed[0].ValueFloat = 200.0f;
	MaxSpeed[1].ValueFloat = 200.0f;
	MaxSpeed[2].ValueFloat = 0.0f;
	MaxSpeed[3].ValueFloat = 200.0f;
	MaxSpeed[4].ValueFloat = 200.0f;

	for (int32 Row = 0; Row < 5; ++Row) Waypoint[Row] = Location[Row];
	Waypoint[3] = FVector(1000.0f, 0.0f, 0.0f);
	Waypoint[4] = FVector(1800.0f, 0.0f, 0.0f);

	Fixture.Movement.ExecuteSystem(0.1f, Fixture.Stores);
	const float HeldX = Location[0].X;

	// Hold push is at most maxSpeed * HoldAvoidanceWeight = 100 cm/s, so at most 10cm in 0.1s.
	// Integrating the stale 200 cm/s walk velocity on this tick would add 20cm and fail the bound.
	TestTrue(TEXT("Hold push is bounded by the cap and ignores stale walking velocity"),
		Location[0].X > 1.0f && Location[0].X <= 10.01f);
	TestTrue(TEXT("Holding agent resists from the other side"), Location[1].X < 20.0f);
	TestTrue(TEXT("Hold push keeps Z untouched"), FMath::IsNearlyEqual(Location[0].Z, 30.0f));
	TestTrue(TEXT("Hold push does not rotate the agent"), FMath::IsNearlyEqual(Rotation[0].Yaw, 45.0f));
	TestTrue(TEXT("Hold push clears stored walking velocity"), Velocity[0].IsNearlyZero());
	TestTrue(TEXT("Dead agent does not slide"), FMath::IsNearlyEqual(Location[2].X, 100.0f));
	TestTrue(TEXT("Dead agent velocity is cleared"), Velocity[2].IsNearlyZero());
	TestTrue(TEXT("Moving agent still steers"), Location[3].X > 500.0f);
	TestTrue(TEXT("Moving agent keeps velocity"), Velocity[3].X > 0.0f);
	// Row 4 starts 90 degrees off its waypoint; the rotation step leaves a 54 degree error, so
	// desired speed is below max speed. The correct desired-speed cap gives ~19.9cm; capping by
	// max speed would give ~23.3cm and fail the bound.
	TestTrue(TEXT("Moving avoidance is capped by desired speed, not max speed"),
		Location[4].X > 800.0f && Location[4].X < 821.5f);

	// Next tick: simulate ability state rewriting the waypoint to the current location.
	for (int32 Row = 0; Row < 3; ++Row) Waypoint[Row] = Location[Row];
	Fixture.Movement.ExecuteSystem(0.1f, Fixture.Stores);

	TestTrue(TEXT("Hold drift stops once separation force is gone"),
		FMath::IsNearlyEqual(Location[0].X, HeldX, 0.01f));

	return true;
}
#endif
