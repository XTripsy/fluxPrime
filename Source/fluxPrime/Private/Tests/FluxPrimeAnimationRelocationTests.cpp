#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "NativeGameplayTags.h"
#include "UObject/UnrealType.h"
#include "Domains/Crowds/Modules/FluxPrimeCrowdsSystemsModule.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RelocationId, "FluxPrime.Tests.Relocation.Id");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RelocationState, "FluxPrime.Tests.Relocation.State");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RelocationAnim, "FluxPrime.Tests.Relocation.Anim");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RelocationTime, "FluxPrime.Tests.Relocation.Time");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RelocationFrame, "FluxPrime.Tests.Relocation.Frame");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RelocationCell, "FluxPrime.Tests.Relocation.Cell");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RelocationLocation, "FluxPrime.Tests.Relocation.Location");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RelocationNotify, "FluxPrime.Tests.Relocation.Notify");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RelocationAlive, "FluxPrime.Tests.Relocation.Alive");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_RelocationPool, "FluxPrime.Tests.Relocation.Pool");

namespace
{
struct FAnimationRelocationFixture
{
	TArray<FFluxPrimeArchetypeDataStore> Stores;
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> Components;
	TArray<TObjectPtr<UFluxPrimeAnimationData>> Animations;
	TArray<TObjectPtr<UFluxPrimeCrowdsIdentity>> Identities;
	TArray<FFluxPrimeArchetypeEntityCatalog> Catalog;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	USystemsConfiguration* Configuration = NewObject<USystemsConfiguration>();
	FFluxPrimeCrowdsSystemsModule Module;

	template<typename T>
	T& Property(UScriptStruct* Type, void* Object, const TCHAR* Name)
	{
		return *FindFProperty<FStructProperty>(Type, Name)->ContainerPtrToValuePtr<T>(Object);
	}

	template<typename T>
	T* Buffer(FGameplayTag Tag)
	{
		return reinterpret_cast<T*>(Stores[0].GetRawBufferData(Stores[0].GetDataID(Tag)));
	}

	void Add(FFluxPrimeDataConfig& Config, FGameplayTag Tag, UScriptStruct* Type)
	{
		Config.EntityDataTag = Tag;
		Config.ExpectedStructType = Type;
		if (Stores[0].GetDataID(Tag) == INDEX_NONE)
			Stores[0].AddBuffer(Tag, Type, Type->GetStructureSize(), 5);
	}

	FAnimationRelocationFixture(bool WithAnimation = true, bool Duplicates = false)
	{
		Stores.SetNum(1);
		Stores[0].SetTotalEntityCount(5);
		Stores[0].SetActiveEntityCount(5);
		Components.Add(NewObject<UInstancedStaticMeshComponent>());
		Components[0]->SetNumCustomDataFloats(7);
		for (int32 Row = 0; Row < 5; ++Row) Components[0]->AddInstance(FTransform::Identity);
		Animations.Add(NewObject<UFluxPrimeAnimationData>());
		Animations[0]->DataAnimations.SetNum(2);
		for (int32 State = 0; State < 2; ++State)
		{
			auto& Definition = Animations[0]->DataAnimations[State];
			Definition.StateTag = State == 0 ? TAG_RelocationState : TAG_RelocationAnim;
			Definition.AnimationStartFrame = State * 100;
			Definition.AnimationEndFrame = State * 100 + 29;
			Definition.AnimationForceNotify = true;
			Definition.AnimationNotifies[0].AnimationNotifyTag = TAG_RelocationNotify;
			Definition.AnimationNotifies[0].AnimationNotifyFrame = State * 100 + 15;
		}
		Identities.Add(NewObject<UFluxPrimeCrowdsIdentity>());
		Catalog.SetNum(1);
		FInstancedStruct AnimationFragment = FInstancedStruct::Make(FFluxPrimeAnimationSystem());
		auto& Animation = AnimationFragment.GetMutable<FFluxPrimeAnimationSystem>();
		auto& A = Property<FAnimationSystemDataTagConfig>(Animation.StaticStruct(), &Animation, TEXT("DataTagConfig"));
		Add(A.EntityDataIdConfig, TAG_RelocationId, FFluxPrimeWrapperInt16::StaticStruct());
		Add(A.EntityDataStateConfig, TAG_RelocationState, FFluxPrimeWrapperInt8::StaticStruct());
		Add(A.EntityDataAnimStateConfig, TAG_RelocationAnim, FFluxPrimeWrapperInt8::StaticStruct());
		Add(A.EntityDataStartTimeAnimConfig, TAG_RelocationTime, FFluxPrimeWrapperFloat::StaticStruct());
		Add(A.EntityDataPreviousFrameAnimConfig, TAG_RelocationFrame, FFluxPrimeWrapperFloat::StaticStruct());
		Add(A.EntityDataCellIdConfig, TAG_RelocationCell, FFluxPrimeWrapperInt32::StaticStruct());
		Add(A.EntityDataTargetLocationConfig, TAG_RelocationLocation, TBaseStructure<FVector>::Get());
		Add(A.EntityDataRequestAnimationNotifyConfig, TAG_RelocationNotify, FFluxPrimeWrapperInt32::StaticStruct());
		auto& Grid = Property<FFluxPrimeSpatialGridSystem>(Animation.StaticStruct(), &Animation, TEXT("SpatialGridSystem"));
		auto& G = Property<FSpatialGridSystemDataTagConfig>(Grid.StaticStruct(), &Grid, TEXT("DataTagConfig"));
		G.EntityDataLocationConfig.EntityDataTag = TAG_RelocationLocation;
		G.EntityDataLocationConfig.ExpectedStructType = TBaseStructure<FVector>::Get();
		G.EntityDataCellIdConfig.EntityDataTag = TAG_RelocationCell;
		G.EntityDataCellIdConfig.ExpectedStructType = FFluxPrimeWrapperInt32::StaticStruct();
		if (WithAnimation)
		{
			Configuration->SystemFragments.Add(AnimationFragment);
			if (Duplicates) Configuration->SystemFragments.Add(AnimationFragment);
		}
		FInstancedStruct CompactFragment = FInstancedStruct::Make(FFluxPrimeCrowdsCompactSystem());
		auto& Compact = CompactFragment.GetMutable<FFluxPrimeCrowdsCompactSystem>();
		auto& C = Property<FCompactSystemDataTagConfig>(Compact.StaticStruct(), &Compact, TEXT("DataTagConfig"));
		Add(C.EntityDataIdConfig, TAG_RelocationId, FFluxPrimeWrapperInt16::StaticStruct());
		Add(C.EntityDataConditionAliveConfig, TAG_RelocationAlive, FFluxPrimeWrapperBool::StaticStruct());
		Add(C.EntityDataRequestBackToPoolConfig, TAG_RelocationPool, FFluxPrimeWrapperBool::StaticStruct());
		Configuration->SystemFragments.Add(CompactFragment);
		if (Duplicates) Configuration->SystemFragments.Add(CompactFragment);
		FindFProperty<FObjectPropertyBase>(Module.StaticStruct(), TEXT("SystemsConfiguration"))
			->SetObjectPropertyValue_InContainer(&Module, Configuration);
		FFluxPrimeCrowdsSystemsModuleContext Context;
		Context.world = World;
		Context.crowdsComponents = &Components;
		Context.resolvedAnimationData = &Animations;
		Context.resolvedIdentities = &Identities;
		Context.entityCatalog = &Catalog;
		Context.dataStores = &Stores;
		Context.hasAuthority = true;
		Context.getStateID = [](FGameplayTag Tag) { return Tag == TAG_RelocationState ? 0 : 1; };
		Context.getNotifyID = [](FGameplayTag) { return 42; };
		Module.InitializeSystems(Context);
		for (int32 Row = 0; Row < 5; ++Row)
		{
			Buffer<FFluxPrimeWrapperBool>(TAG_RelocationAlive)[Row].ValueBool = true;
			Buffer<FFluxPrimeWrapperInt16>(TAG_RelocationId)[Row].ValueInt16 = Row;
			Buffer<FFluxPrimeWrapperFloat>(TAG_RelocationTime)[Row].ValueFloat = -0.625f;
			Buffer<FFluxPrimeWrapperFloat>(TAG_RelocationFrame)[Row].ValueFloat = 18.75f;
			Buffer<FFluxPrimeWrapperInt32>(TAG_RelocationNotify)[Row].ValueInt32 = INDEX_NONE;
			for (int32 Field = 0; Field < 7; ++Field)
				Components[0]->SetCustomDataValue(Row, Field, Row * 100 + Field + 0.25f);
		}
	}

	void Remove(int32 Row)
	{
		Buffer<FFluxPrimeWrapperBool>(TAG_RelocationAlive)[Row].ValueBool = false;
		Buffer<FFluxPrimeWrapperBool>(TAG_RelocationPool)[Row].ValueBool = true;
	}

	~FAnimationRelocationFixture() { World->DestroyWorld(false); }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeAnimationRelocationTest, "FluxPrime.High.AnimationRelocation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeAnimationRelocationTest::RunTest(const FString& Parameters)
{
	for (bool Duplicates : {false, true})
	{
		FAnimationRelocationFixture F(true, Duplicates);
		F.Remove(0);
		F.Remove(3);
		F.Remove(4);
		F.Module.ExecuteModule(0, F.Stores);
		TestEqual(TEXT("Multiple removals leave two survivors"), F.Stores[0].GetActiveEntityCount(), 2);
		for (int32 Row = 0; Row < 2; ++Row)
		{
			const int32 Id = F.Buffer<FFluxPrimeWrapperInt16>(TAG_RelocationId)[Row].ValueInt16;
			for (int32 Field = 0; Field < 6; ++Field)
				TestEqual(TEXT("Same-frame complete VAT state follows survivor"), F.Components[0]->PerInstanceSMCustomData[Row * 7 + Field], Id * 100 + Field + 0.25f);
			TestEqual(TEXT("Unowned custom data is untouched"), F.Components[0]->PerInstanceSMCustomData[Row * 7 + 6], Row * 100 + 6.25f);
			TestEqual(TEXT("Relocation preserves playback start"), F.Buffer<FFluxPrimeWrapperFloat>(TAG_RelocationTime)[Row].ValueFloat, -0.625f);
			TestEqual(TEXT("Relocation preserves notify phase"), F.Buffer<FFluxPrimeWrapperFloat>(TAG_RelocationFrame)[Row].ValueFloat, 18.75f);
		}
		F.Module.ExecuteModule(0, F.Stores);
		TestEqual(TEXT("Next animation tick retains playback start"), F.Buffer<FFluxPrimeWrapperFloat>(TAG_RelocationTime)[0].ValueFloat, -0.625f);
		TestEqual(TEXT("Next animation tick retains notify phase"), F.Buffer<FFluxPrimeWrapperFloat>(TAG_RelocationFrame)[0].ValueFloat, 18.75f);
		TestEqual(TEXT("No notify replay"), F.Buffer<FFluxPrimeWrapperInt32>(TAG_RelocationNotify)[0].ValueInt32, INDEX_NONE);
		F.Remove(1);
		F.Module.GetSystem<FFluxPrimeCrowdsCompactSystem>()->ExecuteSystem(0, F.Stores);
		TestEqual(TEXT("Tail removal leaves one survivor"), F.Stores[0].GetActiveEntityCount(), 1);
		for (int32 Field = 0; Field < 6; ++Field)
			TestEqual(TEXT("Tail removal leaves survivor VAT untouched"), F.Components[0]->PerInstanceSMCustomData[Field], 200 + Field + 0.25f);
	}
	{
		FAnimationRelocationFixture F;
		F.Buffer<FFluxPrimeWrapperFloat>(TAG_RelocationTime)[4].ValueFloat = -0.875f;
		F.Buffer<FFluxPrimeWrapperFloat>(TAG_RelocationFrame)[4].ValueFloat = 26.25f;
		F.Buffer<FFluxPrimeWrapperInt32>(TAG_RelocationNotify)[4].ValueInt32 = 42;
		FInstancedStruct Copied = FInstancedStruct::Make(*F.Module.GetSystem<FFluxPrimeCrowdsCompactSystem>());
		F.Remove(0);
		Copied.GetMutable<FFluxPrimeCrowdsCompactSystem>().ExecuteSystem(0, F.Stores);
		for (int32 Field = 0; Field < 6; ++Field)
			TestEqual(TEXT("Copied compact fragment retains complete VAT relocation"), F.Components[0]->PerInstanceSMCustomData[Field], 400 + Field + 0.25f);
		TestEqual(TEXT("Moved row retains its playback start"), F.Buffer<FFluxPrimeWrapperFloat>(TAG_RelocationTime)[0].ValueFloat, -0.875f);
		TestEqual(TEXT("Moved row retains its notify phase"), F.Buffer<FFluxPrimeWrapperFloat>(TAG_RelocationFrame)[0].ValueFloat, 26.25f);
		TestEqual(TEXT("Pending notify survives relocation"), F.Buffer<FFluxPrimeWrapperInt32>(TAG_RelocationNotify)[0].ValueInt32, 42);
	}
	TFunction<void(int32, int32, int32)> Detached;
	{
		FAnimationRelocationFixture F;
		Detached = F.Module.GetSystem<FFluxPrimeAnimationSystem>()->CreatePresentationRelocator();
		Detached(0, 4, 4);
		TestEqual(TEXT("Self relocation is a no-op"), F.Components[0]->PerInstanceSMCustomData[28], 400.25f);
		Detached(0, -1, 0);
		Detached(9, 0, 1);
	}
	CollectGarbage(RF_NoFlags);
	Detached(0, 4, 0);
	FAnimationRelocationFixture Headless(false);
	Headless.Remove(0);
	Headless.Module.ExecuteModule(0, Headless.Stores);
	TestEqual(TEXT("Compaction works without animation"), Headless.Stores[0].GetActiveEntityCount(), 4);
	TestEqual(TEXT("Headless compaction moves the last entity"), Headless.Buffer<FFluxPrimeWrapperInt16>(TAG_RelocationId)[0].ValueInt16, static_cast<int16>(4));
	for (int32 Field = 0; Field < 7; ++Field)
		TestEqual(TEXT("Without animation custom data stays untouched"), Headless.Components[0]->PerInstanceSMCustomData[Field], Field + 0.25f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeAnimationTimestampTest, "FluxPrime.High.AnimationTimestamp",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeAnimationTimestampTest::RunTest(const FString& Parameters)
{
	FAnimationRelocationFixture F;
	F.Stores[0].SetActiveEntityCount(1);
	auto* Animation = F.Module.GetSystem<FFluxPrimeAnimationSystem>();
	F.Buffer<FFluxPrimeWrapperFloat>(TAG_RelocationTime)[0].ValueFloat = 70000.625f;
	F.Buffer<FFluxPrimeWrapperInt8>(TAG_RelocationState)[0].ValueInt8 = 1;
	Animation->ExecuteSystem(0, F.Stores);
	TestEqual(TEXT("State change retains full outgoing timestamp"), F.Components[0]->PerInstanceSMCustomData[0], 70000.625f);
	F.Buffer<FFluxPrimeWrapperFloat>(TAG_RelocationTime)[0].ValueFloat = 123.625f;
	F.Buffer<FFluxPrimeWrapperInt8>(TAG_RelocationState)[0].ValueInt8 = 0;
	Animation->ExecuteSystem(0, F.Stores);
	TestEqual(TEXT("Return state change retains fractional outgoing timestamp"), F.Components[0]->PerInstanceSMCustomData[0], 123.625f);
	return true;
}
#endif
