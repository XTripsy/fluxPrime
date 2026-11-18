#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Domains/Crowds/Systems/Notify/FluxPrimeCrowdsNotifySpawnVFX.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NativeGameplayTags.h"
#include "UObject/UObjectIterator.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_NotifyVFXTestLocation, "FluxPrime.Tests.NotifyVFX.Location");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_NotifyVFXTestRequest, "FluxPrime.Tests.NotifyVFX.Request");

namespace
{
constexpr int32 NotifyVFXTestNotifyID = 7;
constexpr int32 NotifyVFXTestEntityCount = 16;

UWorld* FindAutomationEditorWorld()
{
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Editor && Context.World() != nullptr)
		{
			return Context.World();
		}
	}
	return nullptr;
}

template<typename T>
T* AddTestBuffer(FFluxPrimeArchetypeDataStore& Store, FGameplayTag Tag, UScriptStruct* StructType)
{
	if (!Store.AddBuffer(Tag, StructType, sizeof(T), NotifyVFXTestEntityCount)) return nullptr;
	return reinterpret_cast<T*>(Store.GetRawBufferData(Store.GetDataID(Tag)));
}

TArray<UNiagaraComponent*> FindSpawnedComponents(UWorld* World, UNiagaraSystem* Asset)
{
	TArray<UNiagaraComponent*> Components;
	for (TObjectIterator<UNiagaraComponent> It; It; ++It)
	{
		if (It->GetWorld() == World && It->GetAsset() == Asset)
		{
			Components.Add(*It);
		}
	}
	return Components;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeNotifySpawnVFXTest, "FluxPrime.Medium.NotifySpawnVFX",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeNotifySpawnVFXTest::RunTest(const FString& Parameters)
{
	UWorld* World = FindAutomationEditorWorld();
	if (World == nullptr)
	{
		AddError(TEXT("No editor world available for the notify VFX test."));
		return false;
	}

	UNiagaraSystem* VFXSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/N_DummyDead.N_DummyDead"));
	if (VFXSystem == nullptr)
	{
		AddError(TEXT("Unable to load /Game/N_DummyDead for the notify VFX test."));
		return false;
	}

	TArray<FFluxPrimeArchetypeDataStore> Stores;
	Stores.SetNum(1);
	FFluxPrimeArchetypeDataStore& Store = Stores[0];
	Store.SetTotalEntityCount(NotifyVFXTestEntityCount);
	Store.SetActiveEntityCount(NotifyVFXTestEntityCount);

	FVector* Locations = AddTestBuffer<FVector>(Store, TAG_NotifyVFXTestLocation, TBaseStructure<FVector>::Get());
	FFluxPrimeWrapperInt32* Requests = AddTestBuffer<FFluxPrimeWrapperInt32>(Store, TAG_NotifyVFXTestRequest, FFluxPrimeWrapperInt32::StaticStruct());
	if (Locations == nullptr || Requests == nullptr)
	{
		AddError(TEXT("Unable to allocate the notify VFX test buffers."));
		return false;
	}

	for (int32 Index = 0; Index < NotifyVFXTestEntityCount; ++Index)
	{
		Locations[Index] = FVector(1000.0 + Index * 10.0, 2000.0 + Index * 10.0, 300.0);
		Requests[Index].ValueInt32 = NotifyVFXTestNotifyID;
	}

	FFluxPrimeCrowdsNotifySpawnVFX Fragment;
	Fragment.VFXSystem = VFXSystem;
	Fragment.DataTagConfig.EntityDataLocationConfig.EntityDataTag = TAG_NotifyVFXTestLocation;
	Fragment.DataTagConfig.EntityDataLocationConfig.ExpectedStructType = TBaseStructure<FVector>::Get();
	Fragment.DataTagConfig.EntityDataRequestNotifyConfig.EntityDataTag = TAG_NotifyVFXTestRequest;
	Fragment.DataTagConfig.EntityDataRequestNotifyConfig.ExpectedStructType = FFluxPrimeWrapperInt32::StaticStruct();

	FFluxPrimeCrowdsNotifySpawnVFXContext Context;
	Context.world = World;
	Context.dataStores = &Stores;
	Context.getNotifyID = [](FGameplayTag) { return static_cast<int16>(NotifyVFXTestNotifyID); };

	const int32 ComponentsBefore = FindSpawnedComponents(World, VFXSystem).Num();

	Fragment.Initialized(Context);

#if WITH_EDITOR
	TestFalse(TEXT("Loading-phase preparation leaves no pending compile request"), VFXSystem->NeedsRequestCompile());
	TestFalse(TEXT("Loading-phase preparation leaves no active compilation"), VFXSystem->HasActiveCompilations());
#endif
	TestTrue(TEXT("Prepared effect is ready to run before first dispatch"), VFXSystem->IsReadyToRun());

	// Simultaneous multi-entity dispatch: every entity requests the effect in the same frame.
	for (int32 Index = 0; Index < NotifyVFXTestEntityCount; ++Index)
	{
		Fragment.ExecuteNotify(0, static_cast<uint16>(Index), Store);
	}

	TArray<UNiagaraComponent*> Spawned = FindSpawnedComponents(World, VFXSystem);
	TestEqual(TEXT("Every entity spawned one effect"), Spawned.Num() - ComponentsBefore, NotifyVFXTestEntityCount);

	bool bLocationsMatch = true;
	for (int32 Index = 0; Index < NotifyVFXTestEntityCount; ++Index)
	{
		const FVector Expected = Locations[Index] + FVector::UpVector * 150.0;
		bool bFound = false;
		for (const UNiagaraComponent* Component : Spawned)
		{
			if (Component->GetComponentLocation().Equals(Expected, 1.0))
			{
				bFound = true;
				break;
			}
		}
		bLocationsMatch &= bFound;
	}
	TestTrue(TEXT("Each effect spawns at the entity location plus 150cm"), bLocationsMatch);

	bool bRotationsMatch = true;
	for (const UNiagaraComponent* Component : Spawned)
	{
		bRotationsMatch &= Component->GetComponentRotation().Equals(FRotator::ZeroRotator, 0.1);
	}
	TestTrue(TEXT("Each effect keeps the configured rotation"), bRotationsMatch);

	bool bRequestsConsumed = true;
	for (int32 Index = 0; Index < NotifyVFXTestEntityCount; ++Index)
	{
		bRequestsConsumed &= Requests[Index].ValueInt32 == INDEX_NONE;
	}
	TestTrue(TEXT("Dispatch consumes every request exactly once"), bRequestsConsumed);

	// Second pass without new requests: nothing may dispatch again.
	for (int32 Index = 0; Index < NotifyVFXTestEntityCount; ++Index)
	{
		Fragment.ExecuteNotify(0, static_cast<uint16>(Index), Store);
	}
	TestEqual(TEXT("Consumed requests do not dispatch twice"), FindSpawnedComponents(World, VFXSystem).Num() - ComponentsBefore, NotifyVFXTestEntityCount);

	// Repeated in-process triggers: a second wave of requests must dispatch without recompiling.
	for (int32 Index = 0; Index < NotifyVFXTestEntityCount; ++Index)
	{
		Requests[Index].ValueInt32 = NotifyVFXTestNotifyID;
	}
	for (int32 Index = 0; Index < NotifyVFXTestEntityCount; ++Index)
	{
		Fragment.ExecuteNotify(0, static_cast<uint16>(Index), Store);
	}
	TestEqual(TEXT("Repeated requests dispatch again"), FindSpawnedComponents(World, VFXSystem).Num() - ComponentsBefore, NotifyVFXTestEntityCount * 2);

#if WITH_EDITOR
	TestFalse(TEXT("Repeated dispatch never requests a compile"), VFXSystem->NeedsRequestCompile());
	TestFalse(TEXT("Repeated dispatch never leaves an active compilation"), VFXSystem->HasActiveCompilations());
#endif

	for (UNiagaraComponent* Component : FindSpawnedComponents(World, VFXSystem))
	{
		Component->DestroyComponent();
	}

	return true;
}
#endif
