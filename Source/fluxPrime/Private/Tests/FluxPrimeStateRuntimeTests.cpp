#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Domains/Crowds/Data/FluxPrimeCrowdsIdentity.h"
#include "Domains/Crowds/Systems/State/FluxPrimeCrowdsStateMachineSystem.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/FluxPrimeStateIdentity.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_StateRuntimeEntityState, "FluxPrime.Tests.StateRuntime.EntityState");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_StateRuntimeWaypoint, "FluxPrime.Tests.StateRuntime.Waypoint");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_StateRuntimeLocation, "FluxPrime.Tests.StateRuntime.Location");

namespace
{
void FillStateStore(FFluxPrimeArchetypeDataStore& Store)
{
	Store.SetTotalEntityCount(1);
	Store.SetActiveEntityCount(0);
	Store.AddBuffer(TAG_StateRuntimeEntityState, FFluxPrimeWrapperInt8::StaticStruct(), sizeof(FFluxPrimeWrapperInt8), 1);
	Store.AddBuffer(TAG_StateRuntimeWaypoint, TBaseStructure<FVector>::Get(), sizeof(FVector), 1);
	Store.AddBuffer(TAG_StateRuntimeLocation, TBaseStructure<FVector>::Get(), sizeof(FVector), 1);
}

FFluxPrimeCrowdsStateMachineSystemContext MakeStateContext(TArray<FFluxPrimeArchetypeEntityCatalog>& Catalog,
	TArray<FFluxPrimeArchetypeDataStore>& Stores)
{
	FFluxPrimeCrowdsStateMachineSystemContext Context;
	Context.getStateID = [](FGameplayTag) { return 0; };
	Context.entityCatalog = &Catalog;
	Context.dataStores = &Stores;
	return Context;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeStateRuntimeIsolationTest, "FluxPrime.High.StateRuntimeIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeStateRuntimeIsolationTest::RunTest(const FString& Parameters)
{
	UObject* outer = GetTransientPackage();

	UFluxPrimeStateIdentity* stateIdentity = NewObject<UFluxPrimeStateIdentity>(outer);
	FFluxPrimeStateDataEditor entry;
	entry.State.InitializeAs<FFluxPrimeCrowdsAbilityState>();
	entry.Transition.InitializeAs<FFluxPrimeCrowdsStateTransition>();
	stateIdentity->StateData.Add(entry);

	FFluxPrimeCrowdsAbilityState* assetState = &stateIdentity->StateData[0].State.GetMutable<FFluxPrimeCrowdsAbilityState>();
	assetState->DataTagConfig.EntityDataCurrentWaypointLocationConfig.EntityDataTag = TAG_StateRuntimeWaypoint;
	assetState->DataTagConfig.EntityDataCurrentWaypointLocationConfig.ExpectedStructType = TBaseStructure<FVector>::Get();
	assetState->DataTagConfig.EntityDataLocationConfig.EntityDataTag = TAG_StateRuntimeLocation;
	assetState->DataTagConfig.EntityDataLocationConfig.ExpectedStructType = TBaseStructure<FVector>::Get();

	UFluxPrimeCrowdsIdentity* identity = NewObject<UFluxPrimeCrowdsIdentity>(outer);
	identity->Identity = TEXT("StateRuntime");
	identity->StateIdentity = stateIdentity;

	TArray<FFluxPrimeArchetypeEntityCatalog> catalog;
	FFluxPrimeArchetypeEntityCatalog catalogEntry;
	catalogEntry.EntityIdentity = identity;
	catalogEntry.TotalEntityOnPool = 1;
	catalog.Add(catalogEntry);

	TArray<FFluxPrimeArchetypeDataStore> storesA;
	storesA.SetNum(1);
	FillStateStore(storesA[0]);

	TArray<FFluxPrimeArchetypeDataStore> storesB;
	storesB.SetNum(1);
	FillStateStore(storesB[0]);

	FFluxPrimeCrowdsStateMachineSystem systemA;
	systemA.DataTagConfig.EntityDataStateConfig.EntityDataTag = TAG_StateRuntimeEntityState;
	systemA.DataTagConfig.EntityDataStateConfig.ExpectedStructType = FFluxPrimeWrapperInt8::StaticStruct();

	FFluxPrimeCrowdsStateMachineSystem systemB;
	systemB.DataTagConfig = systemA.DataTagConfig;

	systemA.Initialized(MakeStateContext(catalog, storesA));

	TestEqual(TEXT("System A gets one runtime archetype"), systemA.GetRuntimeStateData().Num(), 1);
	TestEqual(TEXT("System A gets one runtime state"), systemA.GetRuntimeStateData()[0].StateData.Num(), 1);

	const FFluxPrimeCrowdsAbilityState* runtimeStateA =
		systemA.GetRuntimeStateData()[0].StateData[0].State.GetPtr<FFluxPrimeCrowdsAbilityState>();
	TestNotNull(TEXT("Runtime ability state exists"), runtimeStateA);
	TestEqual(TEXT("Runtime state resolves one tag per store"), runtimeStateA->EntityDataCurrentWaypointLocationID.Num(), 1);
	TestEqual(TEXT("Runtime state resolves the configured tag"),
		runtimeStateA->EntityDataCurrentWaypointLocationID[0], storesA[0].GetDataID(TAG_StateRuntimeWaypoint));

	TestTrue(TEXT("Runtime state is a copy, not asset memory"),
		static_cast<const void*>(runtimeStateA) != static_cast<const void*>(assetState));
	TestEqual(TEXT("Asset state is not mutated by initialization"),
		assetState->EntityDataCurrentWaypointLocationID.Num(), 0);

	systemA.Initialized(MakeStateContext(catalog, storesA));
	TestEqual(TEXT("Repeated initialization does not duplicate runtime tags"),
		systemA.GetRuntimeStateData()[0].StateData[0].State.GetPtr<FFluxPrimeCrowdsAbilityState>()
			->EntityDataCurrentWaypointLocationID.Num(), 1);

	systemB.Initialized(MakeStateContext(catalog, storesB));
	const FFluxPrimeCrowdsAbilityState* runtimeStateB =
		systemB.GetRuntimeStateData()[0].StateData[0].State.GetPtr<FFluxPrimeCrowdsAbilityState>();
	TestNotNull(TEXT("Second system state exists"), runtimeStateB);
	TestTrue(TEXT("Two systems do not share state structs"), runtimeStateA != runtimeStateB);
	TestEqual(TEXT("Second system resolves its own store"), runtimeStateB->EntityDataCurrentWaypointLocationID[0],
		storesB[0].GetDataID(TAG_StateRuntimeWaypoint));

	return true;
}
#endif
