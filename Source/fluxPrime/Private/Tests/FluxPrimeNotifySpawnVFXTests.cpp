#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Domains/Crowds/Systems/Notify/FluxPrimeCrowdsNotifySpawnVFX.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "NativeGameplayTags.h"
#include "NiagaraComponent.h"
#include "NiagaraDataChannel_Global.h"
#include "NiagaraDataChannel_Islands.h"
#include "UObject/UnrealType.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_NotifyVFXTestLocation, "FluxPrime.Tests.NotifyVFX.Location");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_NotifyVFXTestRequest, "FluxPrime.Tests.NotifyVFX.Request");

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeNotifySpawnVFXTest, "FluxPrime.Medium.NotifySpawnVFX",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeNotifySpawnVFXTest::RunTest(const FString& Parameters)
{
	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Editor) World = Context.World();
	}
	if (!TestNotNull(TEXT("Editor world"), World)) return false;

	TArray<FFluxPrimeArchetypeDataStore> Stores;
	Stores.SetNum(1);
	auto& Store = Stores[0];
	Store.SetTotalEntityCount(2);
	Store.SetActiveEntityCount(2);
	Store.AddBuffer(TAG_NotifyVFXTestLocation, TBaseStructure<FVector>::Get(), sizeof(FVector), 2);
	Store.AddBuffer(TAG_NotifyVFXTestRequest, FFluxPrimeWrapperInt32::StaticStruct(), sizeof(FFluxPrimeWrapperInt32), 2);
	auto* Requests = reinterpret_cast<FFluxPrimeWrapperInt32*>(Store.GetRawBufferData(Store.GetDataID(TAG_NotifyVFXTestRequest)));
	if (!TestNotNull(TEXT("Request buffer"), Requests)) return false;
	Requests[0].ValueInt32 = 7;
	Requests[1].ValueInt32 = 8;

	TestNotNull(TEXT("Notify exposes an editor-selectable Niagara system"),
		FindFProperty<FObjectPropertyBase>(FFluxPrimeCrowdsNotifySpawnVFX::StaticStruct(), TEXT("VFXSystem")));
	FFluxPrimeCrowdsNotifySpawnVFX Fragment;
	Fragment.DataTagConfig.EntityDataLocationConfig.EntityDataTag = TAG_NotifyVFXTestLocation;
	Fragment.DataTagConfig.EntityDataLocationConfig.ExpectedStructType = TBaseStructure<FVector>::Get();
	Fragment.DataTagConfig.EntityDataRequestNotifyConfig.EntityDataTag = TAG_NotifyVFXTestRequest;
	Fragment.DataTagConfig.EntityDataRequestNotifyConfig.ExpectedStructType = FFluxPrimeWrapperInt32::StaticStruct();
	FFluxPrimeCrowdsNotifySpawnVFXContext Context;
	Context.world = World;
	Context.dataStores = &Stores;
	Context.getNotifyID = [](FGameplayTag) { return static_cast<int16>(7); };
	Fragment.Initialized(Context);

	TestNull(TEXT("No implicit channel or legacy system fallback"), Fragment.DataChannel.Get());
	TestEqual(TEXT("Default position variable"), Fragment.PositionVariable, FName(TEXT("Position")));
	TestEqual(TEXT("Height offset preserved"), Fragment.TraceHeightOffset, 150.0f);
	Fragment.ExecuteNotify(0, 0, Store);
	Fragment.ExecuteNotify(0, 1, Store);
	TestEqual(TEXT("Missing channel consumes matching cosmetic event"), Requests[0].ValueInt32, INDEX_NONE);
	TestEqual(TEXT("Unrelated notify remains pending"), Requests[1].ValueInt32, 8);
	Fragment.ExecuteNotify(0, 0, Store);
	TestEqual(TEXT("Consumed event is not retried"), Requests[0].ValueInt32, INDEX_NONE);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeNotifyVFXListenerTest, "FluxPrime.Medium.NotifySpawnVFXListeners",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeNotifyVFXListenerTest::RunTest(const FString& Parameters)
{
	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
		if (Context.WorldType == EWorldType::Editor) World = Context.World();
	if (!TestNotNull(TEXT("Editor world"), World)) return false;
	auto* System = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/N_DummyDead.N_DummyDead"));
	if (!TestNotNull(TEXT("Lifecycle fixture (not an NDC rendering test)"), System)) return false;
	auto* Subsystem = NewObject<UFluxPrimeCrowdsVFXSubsystem>(World);
	const auto MakeChannel = [](bool bPosition, bool bIslands, bool bDuplicate)
	{
		auto* Asset = NewObject<UNiagaraDataChannelAsset>();
		UNiagaraDataChannel* Channel = bIslands
			? static_cast<UNiagaraDataChannel*>(NewObject<UNiagaraDataChannel_Islands>(Asset))
			: NewObject<UNiagaraDataChannel_Global>(Asset);
		FindFieldChecked<FObjectPropertyBase>(Asset->GetClass(), TEXT("DataChannel"))->SetObjectPropertyValue_InContainer(Asset, Channel);
		auto* Property = FindFieldChecked<FArrayProperty>(UNiagaraDataChannel::StaticClass(), TEXT("ChannelVariables"));
		auto& Variables = *Property->ContainerPtrToValuePtr<TArray<FNiagaraDataChannelVariable>>(Channel);
		FNiagaraDataChannelVariable Variable;
		Variable.SetName(TEXT("Position"));
		Variable.SetType(bPosition ? FNiagaraTypeDefinition::GetPositionDef()
			: FNiagaraDataChannelVariable::ToDataChannelType(FNiagaraTypeDefinition::GetVec3Def()));
		Variables.Add(Variable);
		if (bDuplicate) Variables.Add(Variable);
		Channel->GetLayoutInfo();
		return Asset;
	};
	auto* Channel = MakeChannel(true, false, false);
	TestFalse(TEXT("Missing system rejected"), Subsystem->Publish(nullptr, Channel, TEXT("Position"), FVector::ZeroVector));
	TestFalse(TEXT("Missing channel rejected"), Subsystem->Publish(System, nullptr, TEXT("Position"), FVector::ZeroVector));
	TestFalse(TEXT("Empty name rejected"), Subsystem->Publish(System, Channel, NAME_None, FVector::ZeroVector));
	TestFalse(TEXT("Vector is not Position"), Subsystem->Publish(System, MakeChannel(false, false, false), TEXT("Position"), FVector::ZeroVector));
	TestFalse(TEXT("Islands rejected"), Subsystem->Publish(System, MakeChannel(true, true, false), TEXT("Position"), FVector::ZeroVector));
	TestFalse(TEXT("Duplicate variable rejected"), Subsystem->Publish(System, MakeChannel(true, false, true), TEXT("Position"), FVector::ZeroVector));
	TestNull(TEXT("Guards do not create listeners"), Subsystem->FindListener(Channel));
	for (int32 Index = 0; Index < 50; ++Index)
		TestTrue(TEXT("Valid event published"), Subsystem->Publish(System, Channel, TEXT("Position"), FVector(Index, 0, 150)));
	auto* Component = Subsystem->FindListener(Channel);
	if (!TestNotNull(TEXT("Persistent listener created"), Component)) return false;
	TestTrue(TEXT("Listener registered"), Component->IsRegistered());
	Subsystem->Publish(System, Channel, TEXT("Position"), FVector::ZeroVector);
	TestTrue(TEXT("Subsequent event reuses listener"), Component == Subsystem->FindListener(Channel));
	TestFalse(TEXT("Different system on same channel rejected"), Subsystem->Publish(NewObject<UNiagaraSystem>(), Channel, TEXT("Position"), FVector::ZeroVector));
	Component->SetPaused(true);
	TestFalse(TEXT("Custom paused listener rejects publication"), Subsystem->Publish(System, Channel, TEXT("Position"), FVector::ZeroVector));
	TestTrue(TEXT("Custom pause preserved"), Component->IsPaused());
	Component->SetPaused(false);
	Component->UnregisterComponent();
	TestFalse(TEXT("Unregistered listener is not duplicated"), Subsystem->Publish(System, Channel, TEXT("Position"), FVector::ZeroVector));
	TestTrue(TEXT("Unregistered listener identity retained"), Component == Subsystem->FindListener(Channel));
	Subsystem->ShutdownListeners();
	TestNull(TEXT("Shutdown clears listener"), Subsystem->FindListener(Channel));
	TestFalse(TEXT("Shutdown destroys component"), Component->IsRegistered());
	TestTrue(TEXT("Explicit shutdown allows new lifecycle"), Subsystem->Publish(System, Channel, TEXT("Position"), FVector::ZeroVector));
	Subsystem->Deinitialize();
	TestNull(TEXT("Deinitialize clears listener"), Subsystem->FindListener(Channel));
	TestFalse(TEXT("Deinitialized subsystem cannot recreate listener"), Subsystem->Publish(System, Channel, TEXT("Position"), FVector::ZeroVector));
	return true;
}
#endif
