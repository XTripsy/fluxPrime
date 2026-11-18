#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Generics/Systems/FluxPrimeSpatialGridSystem.h"
#include "NativeGameplayTags.h"
#include "UObject/UnrealType.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpatialGridTestLocation, "FluxPrime.Tests.SpatialGrid.Location");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpatialGridTestCellId, "FluxPrime.Tests.SpatialGrid.CellId");

namespace
{
/** Grid geometry the system initializes with: 200 x 200 cells of 100 units from -10000. */
constexpr int32 TestCellWidth = 200;
constexpr int32 TestCellSize = 100;

int32 ExpectedCellId(const FVector& Location)
{
	const int32 CellX = FMath::Clamp(FMath::FloorToInt((Location.X + 10000.0) / TestCellSize), 0, TestCellWidth - 1);
	const int32 CellY = FMath::Clamp(FMath::FloorToInt((Location.Y + 10000.0) / TestCellSize), 0, TestCellWidth - 1);
	return (CellY * TestCellWidth) + CellX;
}

FSpatialGridSystemDataTagConfig* GetTagConfig(FFluxPrimeSpatialGridSystem& Grid)
{
	auto* ConfigProperty = FindFProperty<FStructProperty>(
		FFluxPrimeSpatialGridSystem::StaticStruct(), TEXT("DataTagConfig"));
	return ConfigProperty
		? ConfigProperty->ContainerPtrToValuePtr<FSpatialGridSystemDataTagConfig>(&Grid)
		: nullptr;
}

bool AddGridStore(TArray<FFluxPrimeArchetypeDataStore>& Stores, int32 RowCount)
{
	Stores.AddDefaulted();
	FFluxPrimeArchetypeDataStore& Store = Stores.Last();

	const bool bLocation = Store.AddBuffer(TAG_SpatialGridTestLocation,
		TBaseStructure<FVector>::Get(), sizeof(FVector), RowCount);
	const bool bCellId = Store.AddBuffer(TAG_SpatialGridTestCellId,
		FFluxPrimeWrapperInt32::StaticStruct(), sizeof(FFluxPrimeWrapperInt32), RowCount);

	Store.SetActiveEntityCount(RowCount);
	Store.SetTotalEntityCount(RowCount);
	return bLocation && bCellId;
}

FVector* LocationsOf(FFluxPrimeArchetypeDataStore& Store)
{
	return reinterpret_cast<FVector*>(Store.GetRawBufferData(Store.GetDataID(TAG_SpatialGridTestLocation)));
}

FFluxPrimeWrapperInt32* CellIdsOf(FFluxPrimeArchetypeDataStore& Store)
{
	return reinterpret_cast<FFluxPrimeWrapperInt32*>(Store.GetRawBufferData(Store.GetDataID(TAG_SpatialGridTestCellId)));
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeSpatialGridLayoutTest, "FluxPrime.Critical.SpatialGridLayout",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

/**
 * The boids separation pass and the raycast system read GridCounts and GridOffsets after
 * UpdateSpatialGridSystem returns: they fetch the count of a cell and only then the offset.
 * A grid that zeroes its counts when the rebuild ends makes every one of those consumers see
 * an empty world, which is exactly what silently disabled boids separation once.
 */
bool FFluxPrimeSpatialGridLayoutTest::RunTest(const FString& Parameters)
{
	TArray<FFluxPrimeArchetypeDataStore> Stores;
	TestTrue(TEXT("First store registers grid buffers"), AddGridStore(Stores, 4));
	TestTrue(TEXT("Second store registers grid buffers"), AddGridStore(Stores, 1));

	FFluxPrimeSpatialGridSystem Grid;
	FSpatialGridSystemDataTagConfig* Config = GetTagConfig(Grid);
	if (!TestNotNull(TEXT("Grid exposes its tag config"), Config)) return false;

	Config->EntityDataLocationConfig.EntityDataTag = TAG_SpatialGridTestLocation;
	Config->EntityDataLocationConfig.ExpectedStructType = TBaseStructure<FVector>::Get();
	Config->EntityDataCellIdConfig.EntityDataTag = TAG_SpatialGridTestCellId;
	Config->EntityDataCellIdConfig.ExpectedStructType = FFluxPrimeWrapperInt32::StaticStruct();

	Grid.Initialized(Stores);
	TestEqual(TEXT("Grid allocates one count per cell"),
		Grid.GetGridCounts().Num(), TestCellWidth * TestCellWidth);
	TestEqual(TEXT("Grid allocates one offset per cell"),
		Grid.GetGridOffsets().Num(), TestCellWidth * TestCellWidth);

	// Three rows of the first archetype plus all of the second share one cell; the fourth
	// row of the first archetype sits in the next cell along.
	FVector* FirstLocations = LocationsOf(Stores[0]);
	FirstLocations[0] = FVector(0.0, 0.0, 0.0);
	FirstLocations[1] = FVector(10.0, 0.0, 0.0);
	FirstLocations[2] = FVector(20.0, 0.0, 0.0);
	FirstLocations[3] = FVector(150.0, 0.0, 0.0);
	LocationsOf(Stores[1])[0] = FVector(0.0, 0.0, 0.0);

	const int32 SharedCell = ExpectedCellId(FVector(0.0, 0.0, 0.0));
	const int32 SecondCell = ExpectedCellId(FVector(150.0, 0.0, 0.0));

	Grid.UpdateSpatialGridSystem(Stores);

	TestEqual(TEXT("Shared cell id is the expected one"), SharedCell, 20100);
	TestEqual(TEXT("Second cell id is the expected one"), SecondCell, 20101);
	TestEqual(TEXT("Cell id buffer is written for the first archetype"),
		CellIdsOf(Stores[0])[0].ValueInt32, SharedCell);
	TestEqual(TEXT("Cell id buffer is written for the last row"),
		CellIdsOf(Stores[0])[3].ValueInt32, SecondCell);

	// The regression this test exists for: counts must survive the rebuild.
	TestEqual(TEXT("Shared cell keeps its member count"),
		Grid.GetGridCounts()[SharedCell], 4);
	TestEqual(TEXT("Second cell keeps its member count"),
		Grid.GetGridCounts()[SecondCell], 1);
	TestEqual(TEXT("An unused cell stays empty"), Grid.GetGridCounts()[0], 0);

	TestEqual(TEXT("First occupied cell starts at offset zero"),
		Grid.GetGridOffsets()[SharedCell], 0);
	TestEqual(TEXT("Second occupied cell follows the first"),
		Grid.GetGridOffsets()[SecondCell], 4);

	const TArray<FFluxPrimeGridMemberData>& Members = Grid.GetGridMembers();
	TestEqual(TEXT("Member array covers every active row"), Members.Num(), 5);

	// Members of a cell keep (archetype, row) order so the boids own-cell half scan is exhaustive.
	const int32 ExpectedTypes[4] = { 0, 0, 0, 1 };
	const int32 ExpectedRows[4] = { 0, 1, 2, 0 };
	for (int32 Index = 0; Index < 4; ++Index)
	{
		TestEqual(TEXT("Shared cell member type"), Members[Index].EntityType, ExpectedTypes[Index]);
		TestEqual(TEXT("Shared cell member row"), Members[Index].EntityIndex, ExpectedRows[Index]);
	}
	TestEqual(TEXT("Second cell member type"), Members[4].EntityType, 0);
	TestEqual(TEXT("Second cell member row"), Members[4].EntityIndex, 3);

	TestTrue(TEXT("Member caches its location X"),
		FMath::IsNearlyEqual(Members[1].LocationX, 10.0f, 0.001f));
	TestTrue(TEXT("Member caches its location Y"),
		FMath::IsNearlyEqual(Members[1].LocationY, 0.0f, 0.001f));

	// Second rebuild: the previous counts must not leak into the new layout.
	const int32 MovedCell = ExpectedCellId(FVector(5000.0, 0.0, 0.0));
	FirstLocations[0] = FVector(5000.0, 0.0, 0.0);

	Grid.UpdateSpatialGridSystem(Stores);

	TestEqual(TEXT("Source cell drops one member"), Grid.GetGridCounts()[SharedCell], 3);
	TestEqual(TEXT("Second cell keeps its member"), Grid.GetGridCounts()[SecondCell], 1);
	TestEqual(TEXT("Destination cell gains one member"), Grid.GetGridCounts()[MovedCell], 1);
	TestEqual(TEXT("An unused cell stays empty after a rebuild"), Grid.GetGridCounts()[0], 0);

	TestEqual(TEXT("Layout is unchanged for the source cell"),
		Grid.GetGridOffsets()[SharedCell], 0);
	TestEqual(TEXT("Second cell moves up by the dropped member"),
		Grid.GetGridOffsets()[SecondCell], 3);
	TestEqual(TEXT("Destination cell follows the second cell"),
		Grid.GetGridOffsets()[MovedCell], 4);

	TestEqual(TEXT("Moved row is now the only member of its cell"),
		Grid.GetGridMembers()[4].EntityIndex, 0);

	return true;
}
#endif
