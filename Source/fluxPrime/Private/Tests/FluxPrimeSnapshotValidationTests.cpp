#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Domains/Crowds/Systems/FluxPrimeCrowdsSnapshotSystem.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperBool.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SnapshotValidationId, "FluxPrime.Tests.SnapshotValidation.Id");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SnapshotValidationCell, "FluxPrime.Tests.SnapshotValidation.Cell");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SnapshotValidationState, "FluxPrime.Tests.SnapshotValidation.State");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SnapshotValidationLocation, "FluxPrime.Tests.SnapshotValidation.Location");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SnapshotValidationWaypoint, "FluxPrime.Tests.SnapshotValidation.Waypoint");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SnapshotValidationTarget, "FluxPrime.Tests.SnapshotValidation.Target");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SnapshotValidationAlive, "FluxPrime.Tests.SnapshotValidation.Alive");

namespace
{
struct FSnapshotValidationFixture
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TArray<FFluxPrimeArchetypeDataStore> Stores;
	FFluxPrimeCrowdsSnapshotSystem System;
	FOnCrowdsManagerActionChange Action;

	template<typename T>
	T& Property(UScriptStruct* Type, void* Object, const TCHAR* Name)
	{
		return *FindFProperty<FStructProperty>(Type, Name)->ContainerPtrToValuePtr<T>(Object);
	}

	void AddBuffer(FFluxPrimeArchetypeDataStore& Store, FGameplayTag Tag, UScriptStruct* Type)
	{
		Store.AddBuffer(Tag, Type, Type->GetStructureSize(), 4);
	}

	FSnapshotValidationFixture()
	{
		Stores.SetNum(2);

		for (FFluxPrimeArchetypeDataStore& Store : Stores)
		{
			Store.SetTotalEntityCount(4);
			Store.SetActiveEntityCount(4);
			AddBuffer(Store, TAG_SnapshotValidationId, FFluxPrimeWrapperInt16::StaticStruct());
			AddBuffer(Store, TAG_SnapshotValidationCell, FFluxPrimeWrapperInt32::StaticStruct());
			AddBuffer(Store, TAG_SnapshotValidationState, FFluxPrimeWrapperInt8::StaticStruct());
			AddBuffer(Store, TAG_SnapshotValidationLocation, TBaseStructure<FVector>::Get());
			AddBuffer(Store, TAG_SnapshotValidationWaypoint, TBaseStructure<FVector>::Get());
			AddBuffer(Store, TAG_SnapshotValidationTarget, TBaseStructure<FVector>::Get());
			AddBuffer(Store, TAG_SnapshotValidationAlive, FFluxPrimeWrapperBool::StaticStruct());
		}

		auto& Config = Property<FSnapshotSystemDataTagConfig>(System.StaticStruct(), &System, TEXT("DataTagConfig"));
		Config.EntityDataIdConfig = { TAG_SnapshotValidationId, FFluxPrimeWrapperInt16::StaticStruct() };
		Config.EntityDataCellIdConfig = { TAG_SnapshotValidationCell, FFluxPrimeWrapperInt32::StaticStruct() };
		Config.EntityDataStateConfig = { TAG_SnapshotValidationState, FFluxPrimeWrapperInt8::StaticStruct() };
		Config.EntityDataLocationConfig = { TAG_SnapshotValidationLocation, TBaseStructure<FVector>::Get() };
		Config.EntityDataCurrentWaypointLocationConfig = { TAG_SnapshotValidationWaypoint, TBaseStructure<FVector>::Get() };
		Config.EntityDataTargetLocationConfig = { TAG_SnapshotValidationTarget, TBaseStructure<FVector>::Get() };
		Config.EntityDataConditionAliveConfig = { TAG_SnapshotValidationAlive, FFluxPrimeWrapperBool::StaticStruct() };

		auto& Grid = Property<FFluxPrimeSpatialGridSystem>(System.StaticStruct(), &System, TEXT("SpatialGridSystems"));
		auto& GridConfig = Property<FSpatialGridSystemDataTagConfig>(Grid.StaticStruct(), &Grid, TEXT("DataTagConfig"));
		GridConfig.EntityDataLocationConfig.EntityDataTag = TAG_SnapshotValidationLocation;
		GridConfig.EntityDataLocationConfig.ExpectedStructType = TBaseStructure<FVector>::Get();
		GridConfig.EntityDataCellIdConfig.EntityDataTag = TAG_SnapshotValidationCell;
		GridConfig.EntityDataCellIdConfig.ExpectedStructType = FFluxPrimeWrapperInt32::StaticStruct();

		FFluxPrimeSnapshotSystemContext Context;
		Context.world = World;
		Context.onManagerActionChange = &Action;
		Context.dataStores = &Stores;
		Context.hasAuthority = false;
		System.Initialized(Context);

		for (int32 Type = 0; Type < Stores.Num(); ++Type)
		{
			for (int32 Row = 0; Row < 4; ++Row)
			{
				Waypoint(Type, Row) = FVector(1000 * Type + Row, 2000, 3000);
				State(Type, Row).ValueInt8 = 2;
				Alive(Type, Row).ValueBool = true;
			}
		}
	}

	~FSnapshotValidationFixture()
	{
		World->DestroyWorld(false);
	}

	FVector& Waypoint(int32 Type, int32 Row)
	{
		return reinterpret_cast<FVector*>(Stores[Type].GetRawBufferData(
			Stores[Type].GetDataID(TAG_SnapshotValidationWaypoint)))[Row];
	}

	FFluxPrimeWrapperInt8& State(int32 Type, int32 Row)
	{
		return reinterpret_cast<FFluxPrimeWrapperInt8*>(Stores[Type].GetRawBufferData(
			Stores[Type].GetDataID(TAG_SnapshotValidationState)))[Row];
	}

	FFluxPrimeWrapperBool& Alive(int32 Type, int32 Row)
	{
		return reinterpret_cast<FFluxPrimeWrapperBool*>(Stores[Type].GetRawBufferData(
			Stores[Type].GetDataID(TAG_SnapshotValidationAlive)))[Row];
	}

	FInstancedStruct MakeEntry(int32 Type, int32 Row)
	{
		FFluxPrimeCrowdsSnapshotDataPayload Payload;
		FFluxPrimeCrowdsSnapshotData Entry;
		Entry.CrowdType = Type;
		Entry.CrowdIndex = Row;
		Entry.CrowdState = 3;
		Entry.CrowdConditionAlive = false;
		Entry.CrowdNewWaypointLocation = FVector(7, 8, 9);
		Payload.SnapshotDatas.Add(Entry);
		return FInstancedStruct::Make(Payload);
	}

	FInstancedStruct MakeCounts(const TArray<int32>& Counts)
	{
		FFluxPrimeCrowdsSnapshotActiveEntityPayload Payload;
		Payload.SnapshotActiveEntityCountDatas = Counts;
		return FInstancedStruct::Make(Payload);
	}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeSnapshotValidationTest, "FluxPrime.High.SnapshotValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeSnapshotValidationTest::RunTest(const FString& Parameters)
{
	FSnapshotValidationFixture Fixture;

	Fixture.System.SyncData(Fixture.Stores, Fixture.MakeEntry(1, 3));
	TestTrue(TEXT("Valid snapshot entry still applies"), Fixture.Waypoint(1, 3).Equals(FVector(7, 8, 9)));
	TestEqual(TEXT("Valid snapshot state applies"), Fixture.State(1, 3).ValueInt8, 3);
	TestFalse(TEXT("Valid snapshot alive flag applies"), Fixture.Alive(1, 3).ValueBool);

	const FVector TypeSentinel = Fixture.Waypoint(0, 0);
	const int8 StateSentinel = Fixture.State(0, 0).ValueInt8;
	const bool AliveSentinel = Fixture.Alive(0, 0).ValueBool;

	AddExpectedError(TEXT("Snapshot system rejected"), EAutomationExpectedErrorFlags::Contains, 1);
	Fixture.System.SyncData(Fixture.Stores, Fixture.MakeEntry(2, 0));
	TestTrue(TEXT("Out-of-range archetype is rejected"), Fixture.Waypoint(0, 0).Equals(TypeSentinel));
	TestEqual(TEXT("Out-of-range archetype leaves state"), Fixture.State(0, 0).ValueInt8, StateSentinel);
	TestEqual(TEXT("Out-of-range archetype leaves alive flag"), Fixture.Alive(0, 0).ValueBool, AliveSentinel);

	AddExpectedError(TEXT("Snapshot system rejected"), EAutomationExpectedErrorFlags::Contains, 2);
	Fixture.System.SyncData(Fixture.Stores, Fixture.MakeEntry(0, 4));
	Fixture.System.SyncData(Fixture.Stores, Fixture.MakeEntry(0, -1));
	TestTrue(TEXT("Out-of-range rows are rejected"), Fixture.Waypoint(0, 0).Equals(TypeSentinel));
	TestEqual(TEXT("Out-of-range rows leave state"), Fixture.State(0, 0).ValueInt8, StateSentinel);

	AddExpectedError(TEXT("Snapshot system rejected"), EAutomationExpectedErrorFlags::Contains, 1);
	Fixture.System.SyncData(Fixture.Stores, Fixture.MakeCounts({ 4, 4, 4 }));
	TestEqual(TEXT("Oversized count payload is rejected"), Fixture.Stores[0].GetActiveEntityCount(), 4);
	TestEqual(TEXT("Oversized count payload leaves later stores untouched"), Fixture.Stores[1].GetActiveEntityCount(), 4);

	AddExpectedError(TEXT("Snapshot system rejected"), EAutomationExpectedErrorFlags::Contains, 1);
	Fixture.System.SyncData(Fixture.Stores, Fixture.MakeCounts({ 4, 5 }));
	TestEqual(TEXT("Count above pool is rejected before any write"), Fixture.Stores[0].GetActiveEntityCount(), 4);
	TestEqual(TEXT("Count above pool leaves later stores untouched"), Fixture.Stores[1].GetActiveEntityCount(), 4);

	AddExpectedError(TEXT("Snapshot system rejected"), EAutomationExpectedErrorFlags::Contains, 1);
	Fixture.System.SyncData(Fixture.Stores, Fixture.MakeCounts({ -1, 4 }));
	TestEqual(TEXT("Negative count is rejected"), Fixture.Stores[0].GetActiveEntityCount(), 4);

	Fixture.System.SyncData(Fixture.Stores, Fixture.MakeCounts({ 1, 4 }));
	TestEqual(TEXT("Valid count payload applies"), Fixture.Stores[0].GetActiveEntityCount(), 1);
	TestEqual(TEXT("Valid count payload keeps later stores"), Fixture.Stores[1].GetActiveEntityCount(), 4);

	// The configured pool can exceed the buffers actually allocated; trust the allocation, not the count.
	Fixture.Stores[0].SetTotalEntityCount(6);
	Fixture.Stores[0].SetActiveEntityCount(4);
	const FVector BufferSentinel = Fixture.Waypoint(0, 3);

	AddExpectedError(TEXT("Snapshot system rejected"), EAutomationExpectedErrorFlags::Contains, 1);
	Fixture.System.SyncData(Fixture.Stores, Fixture.MakeEntry(0, 4));
	TestTrue(TEXT("Row past the allocated buffer is rejected"), Fixture.Waypoint(0, 3).Equals(BufferSentinel));

	AddExpectedError(TEXT("Snapshot system rejected"), EAutomationExpectedErrorFlags::Contains, 1);
	Fixture.System.SyncData(Fixture.Stores, Fixture.MakeCounts({ 5, 4 }));
	TestEqual(TEXT("Count past the allocated buffer is rejected"), Fixture.Stores[0].GetActiveEntityCount(), 4);

	return true;
}
#endif
