#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Generics/Data/FluxPrimeAnimationIdentity.h"
#include "Generics/Systems/Animation/FluxPrimeAnimationSystem.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperBool.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "NativeGameplayTags.h"
#include "UObject/UnrealType.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationMappingWalk, "FluxPrime.Tests.AnimationMapping.Walk");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationMappingAttack, "FluxPrime.Tests.AnimationMapping.Attack");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationMappingMissing, "FluxPrime.Tests.AnimationMapping.Missing");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationMappingNotify, "FluxPrime.Tests.AnimationMapping.Notify");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationMappingId, "FluxPrime.Tests.AnimationMapping.Id");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationMappingState, "FluxPrime.Tests.AnimationMapping.State");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationMappingAnimState, "FluxPrime.Tests.AnimationMapping.AnimState");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationMappingStart, "FluxPrime.Tests.AnimationMapping.Start");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationMappingFrame, "FluxPrime.Tests.AnimationMapping.Frame");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationMappingCell, "FluxPrime.Tests.AnimationMapping.Cell");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationMappingTarget, "FluxPrime.Tests.AnimationMapping.Target");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_AnimationMappingRequest, "FluxPrime.Tests.AnimationMapping.Request");

namespace
{
struct FAnimationMappingFixture
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TArray<FFluxPrimeArchetypeDataStore> Stores;
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> Components;
	TArray<TObjectPtr<UFluxPrimeAnimationData>> Animations;
	FFluxPrimeAnimationSystem System;

	template<typename T>
	T& Property(UScriptStruct* Type, void* Object, const TCHAR* Name)
	{
		return *FindFProperty<FStructProperty>(Type, Name)->ContainerPtrToValuePtr<T>(Object);
	}

	void Add(FFluxPrimeDataConfig& Config, FGameplayTag Tag, UScriptStruct* Type)
	{
		Config.EntityDataTag = Tag;
		Config.ExpectedStructType = Type;
		Stores[0].AddBuffer(Tag, Type, Type->GetStructureSize(), 1);
	}

	FAnimationMappingFixture(bool MissingState)
	{
		Stores.SetNum(1);
		Stores[0].SetTotalEntityCount(1);
		Stores[0].SetActiveEntityCount(1);

		auto& Config = Property<FAnimationSystemDataTagConfig>(System.StaticStruct(), &System, TEXT("DataTagConfig"));
		Add(Config.EntityDataIdConfig, TAG_AnimationMappingId, FFluxPrimeWrapperInt16::StaticStruct());
		Add(Config.EntityDataStateConfig, TAG_AnimationMappingState, FFluxPrimeWrapperInt8::StaticStruct());
		Add(Config.EntityDataAnimStateConfig, TAG_AnimationMappingAnimState, FFluxPrimeWrapperInt8::StaticStruct());
		Add(Config.EntityDataStartTimeAnimConfig, TAG_AnimationMappingStart, FFluxPrimeWrapperFloat::StaticStruct());
		Add(Config.EntityDataPreviousFrameAnimConfig, TAG_AnimationMappingFrame, FFluxPrimeWrapperFloat::StaticStruct());
		Add(Config.EntityDataCellIdConfig, TAG_AnimationMappingCell, FFluxPrimeWrapperInt32::StaticStruct());
		Add(Config.EntityDataTargetLocationConfig, TAG_AnimationMappingTarget, TBaseStructure<FVector>::Get());
		Add(Config.EntityDataRequestAnimationNotifyConfig, TAG_AnimationMappingRequest, FFluxPrimeWrapperInt32::StaticStruct());

		auto& Grid = Property<FFluxPrimeSpatialGridSystem>(System.StaticStruct(), &System, TEXT("SpatialGridSystem"));
		auto& GridConfig = Property<FSpatialGridSystemDataTagConfig>(Grid.StaticStruct(), &Grid, TEXT("DataTagConfig"));
		GridConfig.EntityDataLocationConfig.EntityDataTag = TAG_AnimationMappingTarget;
		GridConfig.EntityDataLocationConfig.ExpectedStructType = TBaseStructure<FVector>::Get();
		GridConfig.EntityDataCellIdConfig.EntityDataTag = TAG_AnimationMappingCell;
		GridConfig.EntityDataCellIdConfig.ExpectedStructType = FFluxPrimeWrapperInt32::StaticStruct();

		Components.Add(NewObject<UInstancedStaticMeshComponent>());
		Components[0]->SetNumCustomDataFloats(6);
		Components[0]->AddInstance(FTransform::Identity);

		UFluxPrimeAnimationData* Animation = NewObject<UFluxPrimeAnimationData>();
		Animation->DataAnimations.SetNum(2);

		FFluxPrimeAnimationDefinition& Walk = Animation->DataAnimations[0];
		Walk.StateTag = TAG_AnimationMappingWalk;
		Walk.AnimationStartFrame = 0;
		Walk.AnimationEndFrame = 10;
		Walk.AnimationForceNotify = true;
		Walk.AnimationNotifies[0].AnimationNotifyTag = TAG_AnimationMappingNotify;
		Walk.AnimationNotifies[0].AnimationNotifyFrame = 5;

		FFluxPrimeAnimationDefinition& Attack = Animation->DataAnimations[1];
		Attack.StateTag = MissingState ? TAG_AnimationMappingMissing : TAG_AnimationMappingAttack;
		Attack.AnimationStartFrame = 20;
		Attack.AnimationEndFrame = 30;
		Animations.Add(Animation);

		FFluxPrimeAnimationSystemsContext Context;
		Context.world = World;
		Context.componentsISMC = &Components;
		Context.resolvedAnimationData = &Animations;
		Context.dataStores = &Stores;
		Context.globalStateCount = 6;
		Context.getStateID = [](FGameplayTag Tag)
		{
			if (Tag == TAG_AnimationMappingWalk) return 2;
			if (Tag == TAG_AnimationMappingAttack) return 5;
			return static_cast<int32>(INDEX_NONE);
		};
		Context.getNotifyID = [](FGameplayTag) { return 42; };

		System.Initialized(Context);
	}

	~FAnimationMappingFixture()
	{
		World->DestroyWorld(false);
	}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeAnimationPauseClockTest, "FluxPrime.Pause.AnimationClock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeAnimationPauseClockTest::RunTest(const FString& Parameters)
{
	FAnimationMappingFixture Fixture(false);
	auto* State = reinterpret_cast<FFluxPrimeWrapperInt8*>(Fixture.Stores[0].GetRawBufferData(Fixture.Stores[0].GetDataID(TAG_AnimationMappingState)));
	auto* AnimState = reinterpret_cast<FFluxPrimeWrapperInt8*>(Fixture.Stores[0].GetRawBufferData(Fixture.Stores[0].GetDataID(TAG_AnimationMappingAnimState)));
	State[0].ValueInt8 = 2;
	AnimState[0].ValueInt8 = 5;
	Fixture.World->TimeSeconds = 2.0f;
	Fixture.World->RealTimeSeconds = 12.0f;
	Fixture.System.ExecuteSystem(0.1f, Fixture.Stores);
	TestEqual(TEXT("VAT clip starts in world game-time domain"), Fixture.Components[0]->PerInstanceSMCustomData[3], 2.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeAnimationMappingTest, "FluxPrime.High.AnimationMapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeAnimationMappingTest::RunTest(const FString& Parameters)
{
	UObject* outer = GetTransientPackage();

	FFluxPrimeAnimationSystem system;
	TArray<TObjectPtr<UFluxPrimeAnimationData>> resolved;

	TestFalse(TEXT("Empty catalog cannot resolve a mapping"), system.InitializeAnimationMapping(resolved, 4));

	resolved.Add(nullptr);
	AddExpectedError(TEXT("no animation asset is resolved"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("A missing archetype asset aborts mapping"), system.InitializeAnimationMapping(resolved, 4));

	resolved.Reset();
	resolved.Add(NewObject<UFluxPrimeAnimationData>(outer));
	TestTrue(TEXT("A fully resolved archetype set builds a mapping"), system.InitializeAnimationMapping(resolved, 4));
	TestEqual(TEXT("Animation arrays are sized by the global state count"),
		system.GetRuntimeAnimationMapping()[0].AnimationStart.Num(), 4);

	{
		FAnimationMappingFixture Fixture(false);
		TestTrue(TEXT("Sparse global state ids initialize"), Fixture.System.IsAllDataValid);
		const auto& Mapping = Fixture.System.GetRuntimeAnimationMapping()[0];
		TestEqual(TEXT("Mapping is sized to the global state table"), Mapping.AnimationStart.Num(), 6);
		TestEqual(TEXT("Walk resolves to its global id start"), Mapping.AnimationStart[2], 0u);
		TestEqual(TEXT("Walk resolves to its global id end"), Mapping.AnimationEnd[2], 10u);
		TestTrue(TEXT("Walk resolves to its global id notify flag"), Mapping.AnimationForceNotify[2]);
		TestEqual(TEXT("Attack resolves to its global id start"), Mapping.AnimationStart[5], 20u);
		TestEqual(TEXT("Attack resolves to its global id end"), Mapping.AnimationEnd[5], 30u);
		TestEqual(TEXT("Unused global id stays a safe hole"), Mapping.AnimationStart[0], 0u);
		TestEqual(TEXT("Unused global id stays a safe hole end"), Mapping.AnimationEnd[0], 0u);
		TestFalse(TEXT("Unused global id 0 is not an authored animation"), Mapping.AnimationStateValid[0]);
		TestFalse(TEXT("Unused global id 1 is a hole"), Mapping.AnimationStateValid[1]);
		TestTrue(TEXT("Walk marks its global id valid"), Mapping.AnimationStateValid[2]);
		TestFalse(TEXT("Unused global id 3 is a hole"), Mapping.AnimationStateValid[3]);
		TestFalse(TEXT("Unused global id 4 is a hole"), Mapping.AnimationStateValid[4]);
		TestTrue(TEXT("Attack marks its global id valid"), Mapping.AnimationStateValid[5]);
	}

	{
		AddExpectedError(TEXT("is not registered"), EAutomationExpectedErrorFlags::Contains, 1);
		FAnimationMappingFixture Fixture(true);
		TestFalse(TEXT("An unregistered state tag invalidates the animation system"), Fixture.System.IsAllDataValid);
	}

	{
		FFluxPrimeAnimationSystem LimitSystem;
		TArray<TObjectPtr<UFluxPrimeAnimationData>> LimitResolved;
		LimitResolved.Add(NewObject<UFluxPrimeAnimationData>(outer));
		AddExpectedError(TEXT("exceeds the int8 runtime carrier limit"), EAutomationExpectedErrorFlags::Contains, 1);
		TestFalse(TEXT("A global table above the int8 limit is rejected"),
			LimitSystem.InitializeAnimationMapping(LimitResolved, FluxPrimeMaxGlobalStateCount + 1));
	}

	return true;
}
#endif
