#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Domains/Crowds/Data/FluxPrimeCrowdsIdentity.h"
#include "Domains/Crowds/Modules/FluxPrimeCrowdsSpawnerModule.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperBool.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestState, "FluxPrime.Tests.Spawn.State");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestLocation, "FluxPrime.Tests.Spawn.Location");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestRotation, "FluxPrime.Tests.Spawn.Rotation");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestWaypoint, "FluxPrime.Tests.Spawn.Waypoint");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestHealth, "FluxPrime.Tests.Spawn.Health");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestStateData, "FluxPrime.Tests.Spawn.StateData");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestMaxSpeed, "FluxPrime.Tests.Spawn.MaxSpeed");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestCondition, "FluxPrime.Tests.Spawn.Condition");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestRequestPool, "FluxPrime.Tests.Spawn.RequestPool");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestRequestAbility, "FluxPrime.Tests.Spawn.RequestAbility");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestRequestNotify, "FluxPrime.Tests.Spawn.RequestNotify");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestStartTime, "FluxPrime.Tests.Spawn.StartTime");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestPreviousFrame, "FluxPrime.Tests.Spawn.PreviousFrame");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestTargetId, "FluxPrime.Tests.Spawn.TargetId");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestLastOptimize, "FluxPrime.Tests.Spawn.LastOptimize");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestLastMove, "FluxPrime.Tests.Spawn.LastMove");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestCountCorridor, "FluxPrime.Tests.Spawn.CountCorridor");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestNeedReplan, "FluxPrime.Tests.Spawn.NeedReplan");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestCountWaypoints, "FluxPrime.Tests.Spawn.CountWaypoints");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_SpawnTestId, "FluxPrime.Tests.Spawn.Id");

namespace
{
struct FSpawnTestTags
{
	FGameplayTag Location, Rotation, Waypoint, Health, State, MaxSpeed, Condition;
	FGameplayTag RequestPool, RequestAbility, RequestNotify;
	FGameplayTag StartTime, PreviousFrame, TargetId, LastOptimize, LastMove;
	FGameplayTag CountCorridor, NeedReplan, CountWaypoints, Id;
};

FSpawnTestTags MakeTagSet()
{
	return {
		TAG_SpawnTestLocation, TAG_SpawnTestRotation, TAG_SpawnTestWaypoint, TAG_SpawnTestHealth, TAG_SpawnTestStateData,
		TAG_SpawnTestMaxSpeed, TAG_SpawnTestCondition, TAG_SpawnTestRequestPool,
		TAG_SpawnTestRequestAbility, TAG_SpawnTestRequestNotify, TAG_SpawnTestStartTime, TAG_SpawnTestPreviousFrame,
		TAG_SpawnTestTargetId, TAG_SpawnTestLastOptimize, TAG_SpawnTestLastMove, TAG_SpawnTestCountCorridor,
		TAG_SpawnTestNeedReplan, TAG_SpawnTestCountWaypoints, TAG_SpawnTestId
	};
}

void SetSpawnConfig(FFluxPrimeCrowdsSpawnerModule& Module, const FSpawnTestTags& Tags)
{
	auto set = [](FFluxPrimeDataConfig& Config, FGameplayTag Tag, UScriptStruct* Type)
	{
		Config.EntityDataTag = Tag;
		Config.ExpectedStructType = Type;
	};
	set(Module.DataTagConfig.EntityDataLocationConfig, Tags.Location, TBaseStructure<FVector>::Get());
	set(Module.DataTagConfig.EntityDataRotationConfig, Tags.Rotation, TBaseStructure<FRotator>::Get());
	set(Module.DataTagConfig.EntityDataCurrentWaypointLocationConfig, Tags.Waypoint, TBaseStructure<FVector>::Get());
	set(Module.DataTagConfig.EntityDataHealthConfig, Tags.Health, FFluxPrimeWrapperInt16::StaticStruct());
	set(Module.DataTagConfig.EntityDataStateConfig, Tags.State, FFluxPrimeWrapperInt8::StaticStruct());
	set(Module.DataTagConfig.EntityDataMaxSpeedConfig, Tags.MaxSpeed, FFluxPrimeWrapperFloat::StaticStruct());
	set(Module.DataTagConfig.EntityDataConditionConfig, Tags.Condition, FFluxPrimeWrapperBool::StaticStruct());
	set(Module.DataTagConfig.EntityDataRequestBackToPoolConfig, Tags.RequestPool, FFluxPrimeWrapperBool::StaticStruct());
	set(Module.DataTagConfig.EntityDataRequestAbilityConfig, Tags.RequestAbility, FFluxPrimeWrapperBool::StaticStruct());
	set(Module.DataTagConfig.EntityDataRequestAnimationNotifyConfig, Tags.RequestNotify, FFluxPrimeWrapperInt32::StaticStruct());
	set(Module.DataTagConfig.EntityDataStartTimeAnimationConfig, Tags.StartTime, FFluxPrimeWrapperFloat::StaticStruct());
	set(Module.DataTagConfig.EntityDataPreviousAnimationFrameConfig, Tags.PreviousFrame, FFluxPrimeWrapperFloat::StaticStruct());
	set(Module.DataTagConfig.EntityDataTargetIdConfig, Tags.TargetId, FFluxPrimeWrapperInt16::StaticStruct());
	set(Module.DataTagConfig.EntityDataLastOptimizeTimeConfig, Tags.LastOptimize, FFluxPrimeWrapperFloat::StaticStruct());
	set(Module.DataTagConfig.EntityDataLastMoveTargetTimeConfig, Tags.LastMove, FFluxPrimeWrapperFloat::StaticStruct());
	set(Module.DataTagConfig.EntityDataCountCorridorConfig, Tags.CountCorridor, FFluxPrimeWrapperUint8::StaticStruct());
	set(Module.DataTagConfig.EntityDataRequestNeedReplanConfig, Tags.NeedReplan, FFluxPrimeWrapperBool::StaticStruct());
	set(Module.DataTagConfig.EntityDataCountWaypointsConfig, Tags.CountWaypoints, FFluxPrimeWrapperUint8::StaticStruct());
	set(Module.DataTagConfig.EntityDataIdConfig, Tags.Id, FFluxPrimeWrapperInt16::StaticStruct());
}

template<typename T>
void AddSpawnBuffer(FFluxPrimeArchetypeDataStore& Store, FGameplayTag Tag, int32 Count)
{
	Store.AddBuffer(Tag, T::StaticStruct(), sizeof(T), Count);
}

void AddVectorBuffer(FFluxPrimeArchetypeDataStore& Store, FGameplayTag Tag, int32 Count)
{
	Store.AddBuffer(Tag, TBaseStructure<FVector>::Get(), sizeof(FVector), Count);
}

void AddRotatorBuffer(FFluxPrimeArchetypeDataStore& Store, FGameplayTag Tag, int32 Count)
{
	Store.AddBuffer(Tag, TBaseStructure<FRotator>::Get(), sizeof(FRotator), Count);
}

void FillSpawnStore(FFluxPrimeArchetypeDataStore& Store, const FSpawnTestTags& Tags, int32 Count)
{
	Store.SetTotalEntityCount(Count);
	Store.SetActiveEntityCount(0);

	AddVectorBuffer(Store, Tags.Location, Count);
	AddRotatorBuffer(Store, Tags.Rotation, Count);
	AddVectorBuffer(Store, Tags.Waypoint, Count);
	AddSpawnBuffer<FFluxPrimeWrapperInt16>(Store, Tags.Health, Count);
	AddSpawnBuffer<FFluxPrimeWrapperInt8>(Store, Tags.State, Count);
	AddSpawnBuffer<FFluxPrimeWrapperFloat>(Store, Tags.MaxSpeed, Count);
	AddSpawnBuffer<FFluxPrimeWrapperBool>(Store, Tags.Condition, Count);
	AddSpawnBuffer<FFluxPrimeWrapperBool>(Store, Tags.RequestPool, Count);
	AddSpawnBuffer<FFluxPrimeWrapperBool>(Store, Tags.RequestAbility, Count);
	AddSpawnBuffer<FFluxPrimeWrapperInt32>(Store, Tags.RequestNotify, Count);
	AddSpawnBuffer<FFluxPrimeWrapperFloat>(Store, Tags.StartTime, Count);
	AddSpawnBuffer<FFluxPrimeWrapperFloat>(Store, Tags.PreviousFrame, Count);
	AddSpawnBuffer<FFluxPrimeWrapperInt16>(Store, Tags.TargetId, Count);
	AddSpawnBuffer<FFluxPrimeWrapperFloat>(Store, Tags.LastOptimize, Count);
	AddSpawnBuffer<FFluxPrimeWrapperFloat>(Store, Tags.LastMove, Count);
	AddSpawnBuffer<FFluxPrimeWrapperUint8>(Store, Tags.CountCorridor, Count);
	AddSpawnBuffer<FFluxPrimeWrapperBool>(Store, Tags.NeedReplan, Count);
	AddSpawnBuffer<FFluxPrimeWrapperUint8>(Store, Tags.CountWaypoints, Count);
	AddSpawnBuffer<FFluxPrimeWrapperInt16>(Store, Tags.Id, Count);
}

FFluxPrimeCrowdsSpawnerComponentContext MakeContext(UFluxPrimeCrowdsIdentity* Identity,
	TMap<FName, int8>& Types, TArray<FFluxPrimeArchetypeDataStore>& Stores)
{
	FFluxPrimeCrowdsSpawnerComponentContext Context;
	Context.getStateID = [](FGameplayTag) { return 3; };
	Context.crowdsTypes = &Types;
	Context.identity = Identity;
	Context.location = FVector(10.0f, 20.0f, 30.0f);
	Context.rotation = FRotator(0.0f, 45.0f, 0.0f);
	Context.newState = TAG_SpawnTestState;
	Context.dataStores = &Stores;
	return Context;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeSpawnerValidationTest, "FluxPrime.High.SpawnerValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeSpawnerValidationTest::RunTest(const FString& Parameters)
{
	const FSpawnTestTags tags = MakeTagSet();
	UObject* outer = GetTransientPackage();

	UFluxPrimeCrowdsIdentity* identity = NewObject<UFluxPrimeCrowdsIdentity>(outer);
	identity->Identity = TEXT("SpawnTest");
	identity->Speed = 100;
	identity->Health = 50;

	TArray<FFluxPrimeArchetypeDataStore> stores;
	stores.SetNum(1);
	FillSpawnStore(stores[0], tags, 2);

	FFluxPrimeCrowdsSpawnerModule module;
	SetSpawnConfig(module, tags);
	module.Initialized(stores);

	TMap<FName, int8> types;
	types.Add(identity->Identity, 0);

	FFluxPrimeCrowdsSpawnerComponentContext context = MakeContext(identity, types, stores);

	module.SpawnCrowd(context);
	TestEqual(TEXT("Authoritative spawn activates one row"), stores[0].GetActiveEntityCount(), 1);
	auto* location = reinterpret_cast<FVector*>(stores[0].GetRawBufferData(stores[0].GetDataID(tags.Location)));
	auto* health = reinterpret_cast<FFluxPrimeWrapperInt16*>(stores[0].GetRawBufferData(stores[0].GetDataID(tags.Health)));
	auto* speed = reinterpret_cast<FFluxPrimeWrapperFloat*>(stores[0].GetRawBufferData(stores[0].GetDataID(tags.MaxSpeed)));
	auto* condition = reinterpret_cast<FFluxPrimeWrapperBool*>(stores[0].GetRawBufferData(stores[0].GetDataID(tags.Condition)));
	auto* needReplan = reinterpret_cast<FFluxPrimeWrapperBool*>(stores[0].GetRawBufferData(stores[0].GetDataID(tags.NeedReplan)));
	TestTrue(TEXT("Spawn writes the location"), location[0].Equals(context.location));
	TestEqual(TEXT("Spawn writes the identity health"), health[0].ValueInt16, static_cast<int16>(50));
	TestTrue(TEXT("Spawn speed stays within the configured jitter"),
		speed[0].ValueFloat >= 50.0f && speed[0].ValueFloat <= 150.0f);
	TestTrue(TEXT("Spawn marks the entity condition alive"), condition[0].ValueBool);
	TestTrue(TEXT("Spawn requests a navigation replan"), needReplan[0].ValueBool);

	module.SpawnNetCrowd(context);
	TestEqual(TEXT("Replicated spawn activates the next row"), stores[0].GetActiveEntityCount(), 2);

	module.SpawnCrowd(context);
	TestEqual(TEXT("Spawn beyond the pool is rejected"), stores[0].GetActiveEntityCount(), 2);

	FFluxPrimeCrowdsSpawnerComponentContext nullIdentity = MakeContext(nullptr, types, stores);
	AddExpectedError(TEXT("the spawn request has no identity"), EAutomationExpectedErrorFlags::Contains, 1);
	module.SpawnCrowd(nullIdentity);
	TestEqual(TEXT("Null identity does not activate a row"), stores[0].GetActiveEntityCount(), 2);

	FFluxPrimeCrowdsSpawnerModule missingTagModule;
	const FSpawnTestTags missingTags = MakeTagSet();
	SetSpawnConfig(missingTagModule, missingTags);

	TArray<FFluxPrimeArchetypeDataStore> missingStores;
	missingStores.SetNum(1);
	missingStores[0].SetTotalEntityCount(2);
	missingStores[0].SetActiveEntityCount(0);
	AddExpectedError(TEXT("owning module"), EAutomationExpectedErrorFlags::Contains, 1);
	missingTagModule.Initialized(missingStores);

	FFluxPrimeCrowdsSpawnerComponentContext missingContext = MakeContext(identity, types, missingStores);
	missingTagModule.SpawnCrowd(missingContext);
	TestEqual(TEXT("Missing tag rejects the spawn before writing"), missingStores[0].GetActiveEntityCount(), 0);

	module.DataTagConfig.EntityDataLocationConfig.ExpectedStructType = FFluxPrimeWrapperInt8::StaticStruct();
	AddExpectedError(TEXT("owning module"), EAutomationExpectedErrorFlags::Contains, 1);
	module.Initialized(stores);
	stores[0].SetActiveEntityCount(0);
	const FVector sentinel(777, 888, 999);
	location[0] = sentinel;
	module.SpawnCrowd(context);
	TestFalse(TEXT("Wrong buffer type invalidates spawner"), module.IsAllDataValid);
	TestTrue(TEXT("Invalid spawner does not write"), location[0].Equals(sentinel));
	TestEqual(TEXT("Invalid spawner does not activate"), stores[0].GetActiveEntityCount(), 0);
	SetSpawnConfig(module, tags);
	module.Initialized(stores);
	module.Initialized(stores);
	TestTrue(TEXT("Reinitialization recovers validity"), module.IsAllDataValid);
	module.SpawnCrowd(context);
	TestEqual(TEXT("Reinitialized spawner uses current indices"), stores[0].GetActiveEntityCount(), 1);

	// A client has only the shared spawn buffers, not health/navigation buffers.
	TArray<FFluxPrimeArchetypeDataStore> clientStores;
	clientStores.SetNum(1);
	auto& client = clientStores[0];
	client.SetTotalEntityCount(1);
	AddVectorBuffer(client, tags.Location, 1);
	AddRotatorBuffer(client, tags.Rotation, 1);
	AddVectorBuffer(client, tags.Waypoint, 1);
	AddSpawnBuffer<FFluxPrimeWrapperInt8>(client, tags.State, 1);
	AddSpawnBuffer<FFluxPrimeWrapperBool>(client, tags.Condition, 1);
	AddSpawnBuffer<FFluxPrimeWrapperBool>(client, tags.RequestPool, 1);
	AddSpawnBuffer<FFluxPrimeWrapperBool>(client, tags.RequestAbility, 1);
	AddSpawnBuffer<FFluxPrimeWrapperInt32>(client, tags.RequestNotify, 1);
	AddSpawnBuffer<FFluxPrimeWrapperFloat>(client, tags.StartTime, 1);
	AddSpawnBuffer<FFluxPrimeWrapperFloat>(client, tags.PreviousFrame, 1);
	AddSpawnBuffer<FFluxPrimeWrapperInt16>(client, tags.Id, 1);
	module.Initialized(clientStores, false);
	TestTrue(TEXT("Client does not require server-only buffers"), module.IsAllDataValid);
	auto clientContext = MakeContext(identity, types, clientStores);
	module.SpawnCrowd(clientContext);
	TestEqual(TEXT("Client initialization disallows authoritative spawn"), client.GetActiveEntityCount(), 0);
	module.SpawnNetCrowd(clientContext);
	TestEqual(TEXT("Client replicated spawn succeeds"), client.GetActiveEntityCount(), 1);
	return true;
}
#endif
