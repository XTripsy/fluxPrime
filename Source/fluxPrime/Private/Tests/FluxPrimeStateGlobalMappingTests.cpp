#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Domains/Crowds/Data/FluxPrimeCrowdsIdentity.h"
#include "Domains/Crowds/Systems/State/FluxPrimeCrowdsStateMachineSystem.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/FluxPrimeStateData.h"
#include "Framework/Data/FluxPrimeStateIdentity.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_StateMappingIdle, "FluxPrime.Tests.StateMapping.Idle");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_StateMappingWalk, "FluxPrime.Tests.StateMapping.Walk");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_StateMappingUnknown, "FluxPrime.Tests.StateMapping.Unknown");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_StateMappingEntityState, "FluxPrime.Tests.StateMapping.EntityState");

namespace
{
FFluxPrimeStateDataEditor MakeEntry(FGameplayTag StateTag, bool bIdle)
{
	FFluxPrimeStateDataEditor Entry;
	Entry.IdentityState = StateTag;
	if (bIdle) Entry.State.InitializeAs<FFluxPrimeCrowdsIdleState>();
	else Entry.State.InitializeAs<FFluxPrimeCrowdsWalkState>();
	return Entry;
}

UFluxPrimeStateIdentity* MakeStateIdentity(UObject* Outer, const TArray<FFluxPrimeStateDataEditor>& Entries)
{
	UFluxPrimeStateIdentity* Identity = NewObject<UFluxPrimeStateIdentity>(Outer);
	Identity->StateData = Entries;
	return Identity;
}

UFluxPrimeCrowdsIdentity* MakeCrowdIdentity(UObject* Outer, FName Name, UFluxPrimeStateIdentity* StateIdentity)
{
	UFluxPrimeCrowdsIdentity* Identity = NewObject<UFluxPrimeCrowdsIdentity>(Outer);
	Identity->Identity = Name;
	Identity->StateIdentity = StateIdentity;
	return Identity;
}

void FillStore(FFluxPrimeArchetypeDataStore& Store)
{
	Store.SetTotalEntityCount(1);
	Store.SetActiveEntityCount(1);
	Store.AddBuffer(TAG_StateMappingEntityState, FFluxPrimeWrapperInt8::StaticStruct(),
		sizeof(FFluxPrimeWrapperInt8), 1);
}

FFluxPrimeCrowdsStateMachineSystemContext MakeContext(TArray<FFluxPrimeArchetypeEntityCatalog>& Catalog,
	TArray<FFluxPrimeArchetypeDataStore>& Stores, int32 GlobalStateCount, FGetStateID GetStateID)
{
	FFluxPrimeCrowdsStateMachineSystemContext Context;
	Context.getStateID = MoveTemp(GetStateID);
	Context.globalStateCount = GlobalStateCount;
	Context.entityCatalog = &Catalog;
	Context.dataStores = &Stores;
	return Context;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeStateGlobalMappingTest, "FluxPrime.Critical.StateGlobalMapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeStateGlobalMappingTest::RunTest(const FString& Parameters)
{
	UObject* Outer = GetTransientPackage();

	const FGetStateID DenseLookup = [](FGameplayTag Tag)
	{
		if (Tag == TAG_StateMappingIdle) return 0;
		if (Tag == TAG_StateMappingWalk) return 1;
		return static_cast<int32>(INDEX_NONE);
	};

	// The same two states are authored in opposite asset order.
	UFluxPrimeStateIdentity* StateA = MakeStateIdentity(Outer, {
		MakeEntry(TAG_StateMappingIdle, true), MakeEntry(TAG_StateMappingWalk, false) });
	UFluxPrimeStateIdentity* StateB = MakeStateIdentity(Outer, {
		MakeEntry(TAG_StateMappingWalk, false), MakeEntry(TAG_StateMappingIdle, true) });

	TArray<FFluxPrimeArchetypeEntityCatalog> Catalog;
	FFluxPrimeArchetypeEntityCatalog EntryA;
	EntryA.EntityIdentity = MakeCrowdIdentity(Outer, TEXT("A"), StateA);
	EntryA.TotalEntityOnPool = 1;
	Catalog.Add(EntryA);
	FFluxPrimeArchetypeEntityCatalog EntryB;
	EntryB.EntityIdentity = MakeCrowdIdentity(Outer, TEXT("B"), StateB);
	EntryB.TotalEntityOnPool = 1;
	Catalog.Add(EntryB);

	TArray<FFluxPrimeArchetypeDataStore> Stores;
	Stores.SetNum(2);
	FillStore(Stores[0]);
	FillStore(Stores[1]);

	FFluxPrimeCrowdsStateMachineSystem System;
	System.DataTagConfig.EntityDataStateConfig.EntityDataTag = TAG_StateMappingEntityState;
	System.DataTagConfig.EntityDataStateConfig.ExpectedStructType = FFluxPrimeWrapperInt8::StaticStruct();
	System.Initialized(MakeContext(Catalog, Stores, 2, DenseLookup));

	TestTrue(TEXT("Reordered state assets initialize"), System.IsAllDataValid);
	TestEqual(TEXT("One mapping per archetype"), System.GetRuntimeDataStateMachine().Num(), 2);
	TestEqual(TEXT("Executor arrays use the global state count"), System.GetRuntimeDataStateMachine()[0].State.Num(), 2);
	TestEqual(TEXT("Transition arrays use the global state count"), System.GetRuntimeDataStateMachine()[0].Transition.Num(), 2);

	const auto& StateArrayA = System.GetRuntimeDataStateMachine()[0].State;
	const auto& StateArrayB = System.GetRuntimeDataStateMachine()[1].State;
	TestTrue(TEXT("A stores Idle at global id 0"),
		StateArrayA[0].Data && StateArrayA[0].Data->GetScriptStruct() == FFluxPrimeCrowdsIdleState::StaticStruct());
	TestTrue(TEXT("A stores Walk at global id 1"),
		StateArrayA[1].Data && StateArrayA[1].Data->GetScriptStruct() == FFluxPrimeCrowdsWalkState::StaticStruct());
	TestTrue(TEXT("B stores Walk at global id 1 despite reversed asset order"),
		StateArrayB[1].Data && StateArrayB[1].Data->GetScriptStruct() == FFluxPrimeCrowdsWalkState::StaticStruct());
	TestTrue(TEXT("B stores Idle at global id 0 despite reversed asset order"),
		StateArrayB[0].Data && StateArrayB[0].Data->GetScriptStruct() == FFluxPrimeCrowdsIdleState::StaticStruct());
	TestTrue(TEXT("Every registered executor has a dispatch thunk"), StateArrayA[0].Execute != nullptr);

	System.Initialized(MakeContext(Catalog, Stores, 2, DenseLookup));
	TestEqual(TEXT("Repeated initialization does not append executors"),
		System.GetRuntimeDataStateMachine()[0].State.Num(), 2);

	// A sparse global table leaves explicit holes instead of dense shifting.
	UFluxPrimeStateIdentity* StateC = MakeStateIdentity(Outer, { MakeEntry(TAG_StateMappingIdle, true) });
	TArray<FFluxPrimeArchetypeEntityCatalog> SparseCatalog;
	FFluxPrimeArchetypeEntityCatalog EntryC;
	EntryC.EntityIdentity = MakeCrowdIdentity(Outer, TEXT("C"), StateC);
	EntryC.TotalEntityOnPool = 1;
	SparseCatalog.Add(EntryC);

	TArray<FFluxPrimeArchetypeDataStore> SparseStores;
	SparseStores.SetNum(1);
	FillStore(SparseStores[0]);

	const FGetStateID SparseLookup = [](FGameplayTag Tag)
	{
		if (Tag == TAG_StateMappingIdle) return 2;
		if (Tag == TAG_StateMappingWalk) return 5;
		return static_cast<int32>(INDEX_NONE);
	};

	FFluxPrimeCrowdsStateMachineSystem SparseSystem;
	SparseSystem.DataTagConfig = System.DataTagConfig;
	SparseSystem.Initialized(MakeContext(SparseCatalog, SparseStores, 6, SparseLookup));

	TestTrue(TEXT("Sparse state ids initialize"), SparseSystem.IsAllDataValid);
	const auto& SparseStates = SparseSystem.GetRuntimeDataStateMachine()[0].State;
	TestEqual(TEXT("Sparse arrays are sized to the global table"), SparseStates.Num(), 6);
	TestTrue(TEXT("Sparse Idle lands at global id 2"),
		SparseStates[2].Data && SparseStates[2].Data->GetScriptStruct() == FFluxPrimeCrowdsIdleState::StaticStruct());
	TestNull(TEXT("Global id 0 is an explicit hole"), SparseStates[0].Execute);
	TestNull(TEXT("Global id 5 is an explicit hole"), SparseStates[5].Execute);

	// A state that is not in the global table is a named initialization failure.
	UFluxPrimeStateIdentity* StateMissing = MakeStateIdentity(Outer, { MakeEntry(TAG_StateMappingUnknown, true) });
	TArray<FFluxPrimeArchetypeEntityCatalog> MissingCatalog;
	FFluxPrimeArchetypeEntityCatalog EntryMissing;
	EntryMissing.EntityIdentity = MakeCrowdIdentity(Outer, TEXT("Missing"), StateMissing);
	EntryMissing.TotalEntityOnPool = 1;
	MissingCatalog.Add(EntryMissing);

	FFluxPrimeCrowdsStateMachineSystem MissingSystem;
	MissingSystem.DataTagConfig = System.DataTagConfig;
	AddExpectedError(TEXT("is not registered in the global state table"), EAutomationExpectedErrorFlags::Contains, 1);
	MissingSystem.Initialized(MakeContext(MissingCatalog, SparseStores, 6, SparseLookup));
	TestFalse(TEXT("An unregistered state tag invalidates the state machine"), MissingSystem.IsAllDataValid);
	TestEqual(TEXT("Failed initialization leaves no executor table"), MissingSystem.GetRuntimeDataStateMachine().Num(), 0);

	// Authoring the same state tag twice would silently overwrite one executor; it is rejected instead.
	UFluxPrimeStateIdentity* StateDuplicate = MakeStateIdentity(Outer, {
		MakeEntry(TAG_StateMappingIdle, true), MakeEntry(TAG_StateMappingIdle, false) });
	TArray<FFluxPrimeArchetypeEntityCatalog> DuplicateCatalog;
	FFluxPrimeArchetypeEntityCatalog EntryDuplicate;
	EntryDuplicate.EntityIdentity = MakeCrowdIdentity(Outer, TEXT("Duplicate"), StateDuplicate);
	EntryDuplicate.TotalEntityOnPool = 1;
	DuplicateCatalog.Add(EntryDuplicate);

	FFluxPrimeCrowdsStateMachineSystem DuplicateSystem;
	DuplicateSystem.DataTagConfig = System.DataTagConfig;
	AddExpectedError(TEXT("authored more than once"), EAutomationExpectedErrorFlags::Contains, 1);
	DuplicateSystem.Initialized(MakeContext(DuplicateCatalog, SparseStores, 6, SparseLookup));
	TestFalse(TEXT("A duplicate state tag invalidates the state machine"), DuplicateSystem.IsAllDataValid);

	// State ids live in int8 carriers, so a wider global table cannot be addressed safely.
	FFluxPrimeCrowdsStateMachineSystem LimitSystem;
	LimitSystem.DataTagConfig = System.DataTagConfig;
	AddExpectedError(TEXT("exceeds the int8 runtime carrier limit"), EAutomationExpectedErrorFlags::Contains, 1);
	LimitSystem.Initialized(MakeContext(SparseCatalog, SparseStores, FluxPrimeMaxGlobalStateCount + 1, SparseLookup));
	TestFalse(TEXT("A global table above the int8 limit is rejected"), LimitSystem.IsAllDataValid);

	// A failed rebuild must not leave executor pointers into the destroyed runtime copies.
	FFluxPrimeCrowdsStateMachineSystem ReinitSystem;
	ReinitSystem.DataTagConfig = System.DataTagConfig;
	ReinitSystem.Initialized(MakeContext(Catalog, Stores, 2, DenseLookup));
	TestEqual(TEXT("Reinit baseline registers executors"), ReinitSystem.GetRuntimeDataStateMachine().Num(), 2);

	TArray<FFluxPrimeArchetypeEntityCatalog> BrokenCatalog;
	FFluxPrimeArchetypeEntityCatalog EntryBroken;
	BrokenCatalog.Add(EntryBroken);

	AddExpectedError(TEXT("has no StateIdentity"), EAutomationExpectedErrorFlags::Contains, 1);
	ReinitSystem.Initialized(MakeContext(BrokenCatalog, Stores, 2, DenseLookup));
	TestFalse(TEXT("A broken catalog invalidates the state machine"), ReinitSystem.IsAllDataValid);
	TestEqual(TEXT("A failed rebuild drops stale executor tables"),
		ReinitSystem.GetRuntimeDataStateMachine().Num(), 0);

	return true;
}
#endif
