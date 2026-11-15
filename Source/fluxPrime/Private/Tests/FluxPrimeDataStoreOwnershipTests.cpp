#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "NativeGameplayTags.h"
#include <type_traits>

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_DataStoreOwnershipValue, "FluxPrime.Tests.DataStoreOwnership.Value");

static_assert(!std::is_copy_constructible_v<FFluxPrimeArchetypeDataStore>,
	"Owning data stores must not be copy-constructible.");
static_assert(!std::is_copy_assignable_v<FFluxPrimeArchetypeDataStore>,
	"Owning data stores must not be copy-assignable.");
static_assert(std::is_move_constructible_v<FFluxPrimeArchetypeDataStore>,
	"Owning data stores must be movable.");
static_assert(std::is_move_assignable_v<FFluxPrimeArchetypeDataStore>,
	"Owning data stores must be move-assignable.");

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeDataStoreOwnershipTest, "FluxPrime.High.DataStoreOwnership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeDataStoreOwnershipTest::RunTest(const FString& Parameters)
{
	FFluxPrimeArchetypeDataStore source;
	source.SetTotalEntityCount(2);
	TestTrue(TEXT("Owned buffer registers"),
		source.AddBuffer(TAG_DataStoreOwnershipValue, FFluxPrimeWrapperInt32::StaticStruct(),
			sizeof(FFluxPrimeWrapperInt32), 2));

	auto* sourceValues = reinterpret_cast<FFluxPrimeWrapperInt32*>(
		source.GetRawBufferData(source.GetDataID(TAG_DataStoreOwnershipValue)));
	sourceValues[0].ValueInt32 = 11;
	sourceValues[1].ValueInt32 = 22;

	FFluxPrimeArchetypeDataStore moved(MoveTemp(source));

	TestEqual(TEXT("Move transfers the entity count"), moved.GetTotalEntityCount(), 2);
	TestEqual(TEXT("Move transfers the tag mapping"), moved.GetDataID(TAG_DataStoreOwnershipValue), 0);
	auto* movedValues = reinterpret_cast<FFluxPrimeWrapperInt32*>(
		moved.GetRawBufferData(moved.GetDataID(TAG_DataStoreOwnershipValue)));
	TestEqual(TEXT("Move preserves buffer contents"), movedValues[1].ValueInt32, 22);

	TestEqual(TEXT("Move leaves the source without entities"), source.GetTotalEntityCount(), 0);
	TestEqual(TEXT("Move leaves the source without tags"), source.GetDataID(TAG_DataStoreOwnershipValue), INDEX_NONE);

	FFluxPrimeArchetypeDataStore assigned;
	assigned = MoveTemp(moved);

	TestEqual(TEXT("Move assignment transfers the tag mapping"), assigned.GetDataID(TAG_DataStoreOwnershipValue), 0);
	TestEqual(TEXT("Move assignment leaves the source without entities"), moved.GetTotalEntityCount(), 0);

	assigned.FreeManualBuffer();
	assigned.FreeManualBuffer();
	TestEqual(TEXT("Free is idempotent"), assigned.GetDataID(TAG_DataStoreOwnershipValue), INDEX_NONE);
	TestEqual(TEXT("Free clears the entity count"), assigned.GetTotalEntityCount(), 0);

	return true;
}
#endif
