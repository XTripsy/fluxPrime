#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "NativeGameplayTags.h"
#include "Generics/Systems/FluxPrimeMovementSystem.h"
#include "Generics/Systems/FluxPrimeVisualInterpolationSystem.h"
#include "Domains/Crowds/Systems/State/StateLogics/FluxPrimeCrowdsAbilityState.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ReaimAcceleration, "FluxPrime.Tests.Reaim.Acceleration");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ReaimVelocity, "FluxPrime.Tests.Reaim.Velocity");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ReaimSpeed, "FluxPrime.Tests.Reaim.Speed");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ReaimLocation, "FluxPrime.Tests.Reaim.Location");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ReaimWaypoint, "FluxPrime.Tests.Reaim.Waypoint");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ReaimRotation, "FluxPrime.Tests.Reaim.Rotation");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ReaimTargetLocation, "FluxPrime.Tests.Reaim.TargetLocation");

namespace
{
constexpr float ReaimYawTolerance = 0.01f;
const FVector ReaimTestLocation(100.0f, 0.0f, 0.0f);

enum class EReaimAim : uint8 { Full, None, WrongRotationType };

struct FReaimFixture
{
	FFluxPrimeMeshOrientationConfig Orientation;
	TArray<FFluxPrimeArchetypeDataStore> Stores;
	FFluxPrimeCrowdsAbilityState State;

	FReaimFixture(int32 EntityCount, float TurnRate, float YawOffset, EReaimAim Aim = EReaimAim::Full)
	{
		Orientation.ForwardYawOffsetDegrees = YawOffset;
		State.MeshOrientation = &Orientation;
		State.AimTurnRateDegreesPerSecond = TurnRate;

		Stores.SetNum(1);
		FFluxPrimeArchetypeDataStore& Store = Stores[0];
		Store.SetTotalEntityCount(EntityCount);
		Store.SetActiveEntityCount(EntityCount);

		auto Add = [&Store, EntityCount](FGameplayTag Tag, UScriptStruct* Type)
		{
			Store.AddBuffer(Tag, Type, Type->GetStructureSize(), EntityCount);
		};
		auto Config = [](FFluxPrimeDataConfig& Data, FGameplayTag Tag, UScriptStruct* Type)
		{
			Data.EntityDataTag = Tag;
			Data.ExpectedStructType = Type;
		};

		Add(TAG_ReaimLocation, TBaseStructure<FVector>::Get());
		Add(TAG_ReaimWaypoint, TBaseStructure<FVector>::Get());
		Config(State.DataTagConfig.EntityDataLocationConfig, TAG_ReaimLocation, TBaseStructure<FVector>::Get());
		Config(State.DataTagConfig.EntityDataCurrentWaypointLocationConfig, TAG_ReaimWaypoint, TBaseStructure<FVector>::Get());

		if (Aim != EReaimAim::None)
		{
			Add(TAG_ReaimRotation, Aim == EReaimAim::WrongRotationType
				? TBaseStructure<FVector>::Get() : TBaseStructure<FRotator>::Get());
			Add(TAG_ReaimTargetLocation, TBaseStructure<FVector>::Get());
			Config(State.DataTagConfig.EntityDataRotationConfig, TAG_ReaimRotation, TBaseStructure<FRotator>::Get());
			Config(State.DataTagConfig.EntityDataTargetLocationConfig, TAG_ReaimTargetLocation, TBaseStructure<FVector>::Get());
		}

		State.Initialized(Stores, &Orientation);
	}

	template<typename T>
	T* Buffer(FGameplayTag Tag)
	{
		return reinterpret_cast<T*>(Stores[0].GetRawBufferData(Stores[0].GetDataID(Tag)));
	}

	void Run(int32 Entity, float DeltaTime)
	{
		State.ExecuteState(DeltaTime, 0, Entity, Stores[0]);
	}

	void SetRow(int32 Row, FVector Location, float Yaw, FVector Target)
	{
		Buffer<FVector>(TAG_ReaimLocation)[Row] = Location;
		Buffer<FVector>(TAG_ReaimWaypoint)[Row] = FVector::ZeroVector;
		Buffer<FRotator>(TAG_ReaimRotation)[Row] = FRotator(0.0f, Yaw, 0.0f);
		Buffer<FVector>(TAG_ReaimTargetLocation)[Row] = Target;
	}

	float Yaw(int32 Row) const
	{
		return reinterpret_cast<const FRotator*>(Stores[0].GetRawBufferData(Stores[0].GetDataID(TAG_ReaimRotation)))[Row].Yaw;
	}

	bool RowKeptLocationAndHold(int32 Row) const
	{
		const FVector* Location = reinterpret_cast<const FVector*>(Stores[0].GetRawBufferData(Stores[0].GetDataID(TAG_ReaimLocation)));
		const FVector* Waypoint = reinterpret_cast<const FVector*>(Stores[0].GetRawBufferData(Stores[0].GetDataID(TAG_ReaimWaypoint)));
		return Location[Row].Equals(ReaimTestLocation) && Waypoint[Row].Equals(Location[Row]);
	}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeAbilityReaimTest, "FluxPrime.High.AbilityReaim",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeAbilityReaimTest::RunTest(const FString& Parameters)
{
	// Rate clamp, shortest direction, and the 180 degree seam.
	{
		FReaimFixture Fixture(3, 180.0f, 0.0f);
		Fixture.SetRow(0, ReaimTestLocation, 0.0f, FVector(100.0f, 1000.0f, 0.0f));
		Fixture.SetRow(1, ReaimTestLocation, 0.0f, FVector(100.0f, -1000.0f, 0.0f));
		Fixture.SetRow(2, ReaimTestLocation, 179.0f, FVector(100.0f, -1000.0f, 0.0f));

		for (int32 Row = 0; Row < 3; ++Row) Fixture.Run(Row, 0.1f);

		TestTrue(TEXT("Turn is clamped to rate times delta time"), FMath::IsNearlyEqual(Fixture.Yaw(0), 18.0f, ReaimYawTolerance));
		TestTrue(TEXT("Turn takes the negative short way"), FMath::IsNearlyEqual(Fixture.Yaw(1), -18.0f, ReaimYawTolerance));
		TestTrue(TEXT("Turn crosses the seam the short way"), FMath::IsNearlyEqual(Fixture.Yaw(2), 197.0f, ReaimYawTolerance));
		for (int32 Row = 0; Row < 3; ++Row)
		{
			TestTrue(TEXT("Reaim keeps the location and the hold waypoint"), Fixture.RowKeptLocationAndHold(Row));
		}
	}

	// Convergence, no overshoot, Z dropped, zero-length direction.
	{
		FReaimFixture Fixture(3, 1800.0f, 0.0f);
		Fixture.SetRow(0, ReaimTestLocation, 0.0f, FVector(100.0f, 1000.0f, 0.0f));
		Fixture.SetRow(1, ReaimTestLocation, 33.0f, FVector(100.0f, 0.0f, 1000.0f));
		Fixture.SetRow(2, ReaimTestLocation, 44.0f, FVector(100.0f, 0.0f, 0.0f));

		for (int32 Row = 0; Row < 3; ++Row) Fixture.Run(Row, 0.1f);

		TestTrue(TEXT("High turn rate reaches the aim yaw in one tick"), FMath::IsNearlyEqual(Fixture.Yaw(0), 90.0f, ReaimYawTolerance));
		TestTrue(TEXT("Target Z is ignored before the zero-length check"), FMath::IsNearlyEqual(Fixture.Yaw(1), 33.0f, ReaimYawTolerance));
		TestTrue(TEXT("Zero-length direction keeps the facing"), FMath::IsNearlyEqual(Fixture.Yaw(2), 44.0f, ReaimYawTolerance));

		for (int32 Row = 0; Row < 3; ++Row) Fixture.Run(Row, 0.1f);

		TestTrue(TEXT("Converged aim holds without overshoot"), FMath::IsNearlyEqual(Fixture.Yaw(0), 90.0f, ReaimYawTolerance));
		for (int32 Row = 0; Row < 3; ++Row)
		{
			TestTrue(TEXT("Converged reaim keeps the location and the hold waypoint"), Fixture.RowKeptLocationAndHold(Row));
		}
	}

	// Mesh yaw offset.
	{
		FReaimFixture Fixture(1, 1800.0f, 45.0f);
		Fixture.SetRow(0, ReaimTestLocation, 0.0f, FVector(1100.0f, 0.0f, 0.0f));

		Fixture.Run(0, 0.1f);

		TestTrue(TEXT("Aim yaw goes through the mesh orientation offset"), FMath::IsNearlyEqual(Fixture.Yaw(0), 45.0f, ReaimYawTolerance));
		TestTrue(TEXT("Aim keeps the location and the hold waypoint"), Fixture.RowKeptLocationAndHold(0));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeAbilityReaimConfigTest, "FluxPrime.High.AbilityReaimConfig",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeAbilityReaimConfigTest::RunTest(const FString& Parameters)
{
	// No aim config at all: the state still runs its hold write.
	{
		FReaimFixture Fixture(1, 180.0f, 0.0f, EReaimAim::None);
		Fixture.Buffer<FVector>(TAG_ReaimLocation)[0] = ReaimTestLocation;
		Fixture.Buffer<FVector>(TAG_ReaimWaypoint)[0] = FVector(999.0f, 0.0f, 0.0f);

		Fixture.Run(0, 0.1f);

		TestTrue(TEXT("Missing aim config still writes the hold waypoint"),
			Fixture.Buffer<FVector>(TAG_ReaimWaypoint)[0].Equals(ReaimTestLocation));
	}

	// Aim tag configured but the buffer is the wrong struct type.
	{
		FReaimFixture Fixture(1, 180.0f, 0.0f, EReaimAim::WrongRotationType);
		Fixture.Buffer<FVector>(TAG_ReaimLocation)[0] = ReaimTestLocation;
		Fixture.Buffer<FVector>(TAG_ReaimWaypoint)[0] = FVector(999.0f, 0.0f, 0.0f);

		Fixture.Run(0, 0.1f);

		TestTrue(TEXT("Wrongly typed rotation buffer still writes the hold waypoint"),
			Fixture.Buffer<FVector>(TAG_ReaimWaypoint)[0].Equals(ReaimTestLocation));
	}

	// Either half of an opt-in configuration must leave facing untouched.
	for (bool MissingRotation : {false, true})
	{
		FReaimFixture Fixture(1, 180.0f, 0.0f);
		Fixture.SetRow(0, ReaimTestLocation, 30.0f, FVector(100, 1000, 0));
		if (MissingRotation) Fixture.State.DataTagConfig.EntityDataRotationConfig.EntityDataTag = FGameplayTag();
		else Fixture.State.DataTagConfig.EntityDataTargetLocationConfig.EntityDataTag = FGameplayTag();
		Fixture.State.Initialized(Fixture.Stores, &Fixture.Orientation);
		Fixture.Run(0, 0.1f);
		TestTrue(TEXT("Partial aim config keeps facing"), FMath::IsNearlyEqual(Fixture.Yaw(0), 30.0f, ReaimYawTolerance));
		TestTrue(TEXT("Partial aim config still holds"), Fixture.RowKeptLocationAndHold(0));
	}
	for (bool WrongRotation : {false, true})
	{
		FReaimFixture Fixture(1, 180.0f, 0.0f);
		Fixture.SetRow(0, ReaimTestLocation, 30.0f, FVector(100, 1000, 0));
		if (WrongRotation) Fixture.State.DataTagConfig.EntityDataRotationConfig.ExpectedStructType = TBaseStructure<FVector>::Get();
		else Fixture.State.DataTagConfig.EntityDataTargetLocationConfig.ExpectedStructType = TBaseStructure<FRotator>::Get();
		Fixture.State.Initialized(Fixture.Stores, &Fixture.Orientation);
		Fixture.Run(0, 0.1f);
		TestTrue(TEXT("Wrong aim type keeps facing"), FMath::IsNearlyEqual(Fixture.Yaw(0), 30.0f, ReaimYawTolerance));
		TestTrue(TEXT("Wrong aim type keeps target buffer"), Fixture.Buffer<FVector>(TAG_ReaimTargetLocation)[0].Equals(FVector(100, 1000, 0)));
		TestTrue(TEXT("Wrong aim type still holds"), Fixture.RowKeptLocationAndHold(0));
	}

	// Zero turn rate disables reaim.
	{
		FReaimFixture Fixture(1, 0.0f, 0.0f);
		Fixture.SetRow(0, ReaimTestLocation, 30.0f, FVector(100.0f, 1000.0f, 0.0f));

		Fixture.Run(0, 0.1f);

		TestTrue(TEXT("Zero turn rate freezes the facing"), FMath::IsNearlyEqual(Fixture.Yaw(0), 30.0f, ReaimYawTolerance));
		TestTrue(TEXT("Zero turn rate keeps the hold waypoint"), Fixture.RowKeptLocationAndHold(0));
	}

	// Zero delta time rotates nothing.
	{
		FReaimFixture Fixture(1, 180.0f, 0.0f);
		Fixture.SetRow(0, ReaimTestLocation, 30.0f, FVector(100.0f, 1000.0f, 0.0f));

		Fixture.Run(0, 0.0f);

		TestTrue(TEXT("Zero delta time freezes the facing"), FMath::IsNearlyEqual(Fixture.Yaw(0), 30.0f, ReaimYawTolerance));
		TestTrue(TEXT("Zero delta time keeps the hold waypoint"), Fixture.RowKeptLocationAndHold(0));
	}

	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeReaimHoldPipelineTest, "FluxPrime.High.AbilityReaimHoldPipeline",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeReaimHoldPipelineTest::RunTest(const FString& Parameters)
{
	FReaimFixture Fixture(1, 180.0f, 0.0f);
	auto& Store = Fixture.Stores[0];
	Store.AddBuffer(TAG_ReaimAcceleration, TBaseStructure<FVector>::Get(), sizeof(FVector), 1);
	Store.AddBuffer(TAG_ReaimVelocity, TBaseStructure<FVector>::Get(), sizeof(FVector), 1);
	Store.AddBuffer(TAG_ReaimSpeed, FFluxPrimeWrapperFloat::StaticStruct(), sizeof(FFluxPrimeWrapperFloat), 1);
	FFluxPrimeMovementSystem Movement;
	FFluxPrimeVisualInterpolationSystem Interpolation;
	auto Config = [](FFluxPrimeDataConfig& Data, FGameplayTag Tag, UScriptStruct* Type)
	{
		Data.EntityDataTag = Tag;
		Data.ExpectedStructType = Type;
	};
	Config(Movement.DataTagConfig.EntityDataLocationConfig, TAG_ReaimLocation, TBaseStructure<FVector>::Get());
	Config(Movement.DataTagConfig.EntityDataRotationConfig, TAG_ReaimRotation, TBaseStructure<FRotator>::Get());
	Config(Movement.DataTagConfig.EntityDataCurrentWaypointConfig, TAG_ReaimWaypoint, TBaseStructure<FVector>::Get());
	Config(Movement.DataTagConfig.EntityDataAccelerationConfig, TAG_ReaimAcceleration, TBaseStructure<FVector>::Get());
	Config(Movement.DataTagConfig.EntityDataVelocityConfig, TAG_ReaimVelocity, TBaseStructure<FVector>::Get());
	Config(Movement.DataTagConfig.EntityDataMaxSpeedConfig, TAG_ReaimSpeed, FFluxPrimeWrapperFloat::StaticStruct());
	Config(Interpolation.DataTagConfig.EntityDataLocationConfig, TAG_ReaimLocation, TBaseStructure<FVector>::Get());
	Config(Interpolation.DataTagConfig.EntityDataRotationConfig, TAG_ReaimRotation, TBaseStructure<FRotator>::Get());
	Config(Interpolation.DataTagConfig.EntityDataCurrentWaypointLocationConfig, TAG_ReaimWaypoint, TBaseStructure<FVector>::Get());
	Movement.MeshOrientation = &Fixture.Orientation;
	Interpolation.MeshOrientation = &Fixture.Orientation;
	Movement.Initialized(Fixture.Stores);
	Interpolation.Initialized(Fixture.Stores);
	Fixture.SetRow(0, ReaimTestLocation, 0.0f, FVector(100, 1000, 0));
	Fixture.Buffer<FVector>(TAG_ReaimAcceleration)[0] = FVector(0, 500, 0);
	Fixture.Buffer<FVector>(TAG_ReaimVelocity)[0] = FVector(200, 0, 0);
	Fixture.Buffer<FFluxPrimeWrapperFloat>(TAG_ReaimSpeed)[0].ValueFloat = 200;
	Fixture.Run(0, 0.1f);
	Movement.ExecuteSystem(0.1f, Fixture.Stores);
	Interpolation.ExecuteSystem(0.1f, Fixture.Stores);
	TestTrue(TEXT("Interpolation preserves attack facing after separation"), FMath::IsNearlyEqual(Fixture.Yaw(0), 18.0f, 0.01f));
	TestTrue(TEXT("Interpolation preserves the bounded hold push"), Fixture.Buffer<FVector>(TAG_ReaimLocation)[0].Equals(FVector(100, 10, 0)));
	TestTrue(TEXT("Hold has no stale waypoint after separation"), Fixture.Buffer<FVector>(TAG_ReaimWaypoint)[0].Equals(Fixture.Buffer<FVector>(TAG_ReaimLocation)[0]));
	return true;
}
#endif
