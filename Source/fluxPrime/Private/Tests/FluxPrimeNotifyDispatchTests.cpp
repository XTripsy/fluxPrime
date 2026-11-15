#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "NativeGameplayTags.h"
#include "Domains/Crowds/Systems/Notify/FluxPrimeCrowdsNotifyAbility.h"
#include "Generics/Systems/Animation/FluxPrimeBaseAnimationNotifySystem.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_NotifyTestRequest, "FluxPrime.Tests.Notify.Request");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_NotifyTestOtherRequest, "FluxPrime.Tests.Notify.OtherRequest");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_NotifyTestAbility, "FluxPrime.Tests.Notify.Ability");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_NotifyTestOtherAbility, "FluxPrime.Tests.Notify.OtherAbility");

namespace
{
struct FNotifyTestDispatcher : FFluxPrimeBaseAnimationNotifySystem
{
	using FFluxPrimeBaseAnimationNotifySystem::ResolveSharedRequestNotify;
	void Add(FInstancedStruct& Fragment) { RegisterAbility<FFluxPrimeCrowdsNotifyAbility>(Fragment); }
};

template<typename T>
T* AddBuffer(FFluxPrimeArchetypeDataStore& Store, FGameplayTag Tag)
{
	if (!Store.AddBuffer(Tag, T::StaticStruct(), sizeof(T), 1)) return nullptr;
	return reinterpret_cast<T*>(Store.GetRawBufferData(Store.GetDataID(Tag)));
}

void InitNotify(FInstancedStruct& Fragment, TArray<FFluxPrimeArchetypeDataStore>& Stores,
	int32 NotifyID, FGameplayTag RequestTag, FGameplayTag AbilityTag, bool Valid = true)
{
	Fragment.InitializeAs<FFluxPrimeCrowdsNotifyAbility>();
	auto& Notify = Fragment.GetMutable<FFluxPrimeCrowdsNotifyAbility>();
	Notify.DataTagConfig.EntityDataRequestNotifyConfig.EntityDataTag = RequestTag;
	Notify.DataTagConfig.EntityDataRequestNotifyConfig.ExpectedStructType = Valid ? FFluxPrimeWrapperInt32::StaticStruct() : nullptr;
	Notify.DataTagConfig.EntityDataRequestAbilityConfig.EntityDataTag = AbilityTag;
	Notify.DataTagConfig.EntityDataRequestAbilityConfig.ExpectedStructType = FFluxPrimeWrapperBool::StaticStruct();
	FFluxPrimeCrowdsNotifyAbilityContext Context;
	Context.dataStores = &Stores;
	Context.getNotifyID = [NotifyID](FGameplayTag) { return NotifyID; };
	Notify.Initialized(Context);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeNotifyDispatchTest, "FluxPrime.Medium.NotifyDispatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeNotifyDispatchTest::RunTest(const FString& Parameters)
{
	TArray<FFluxPrimeArchetypeDataStore> Stores;
	Stores.SetNum(1);
	auto& Store = Stores[0];
	Store.SetTotalEntityCount(1);
	Store.SetActiveEntityCount(1);
	auto* Request = AddBuffer<FFluxPrimeWrapperInt32>(Store, TAG_NotifyTestRequest);
	auto* OtherRequest = AddBuffer<FFluxPrimeWrapperInt32>(Store, TAG_NotifyTestOtherRequest);
	auto* Ability = AddBuffer<FFluxPrimeWrapperBool>(Store, TAG_NotifyTestAbility);
	auto* OtherAbility = AddBuffer<FFluxPrimeWrapperBool>(Store, TAG_NotifyTestOtherAbility);

	FInstancedStruct First, Second;
	InitNotify(First, Stores, INDEX_NONE, TAG_NotifyTestRequest, TAG_NotifyTestAbility);
	FNotifyTestDispatcher Unresolved;
	Unresolved.Add(First);
	Request->ValueInt32 = INDEX_NONE;
	Unresolved.ExecuteSystem(0, Stores);
	TestTrue(TEXT("Unresolved ID retains legacy empty-request match"), Ability->ValueBool);

	InitNotify(First, Stores, 7, TAG_NotifyTestRequest, TAG_NotifyTestAbility);
	InitNotify(Second, Stores, 7, TAG_NotifyTestRequest, TAG_NotifyTestOtherAbility);
	FNotifyTestDispatcher Duplicates;
	Duplicates.Add(First);
	Duplicates.Add(Second);
	Ability->ValueBool = OtherAbility->ValueBool = false;
	Request->ValueInt32 = 7;
	Duplicates.ExecuteSystem(0, Stores);
	TestTrue(TEXT("First duplicate consumes request"), Ability->ValueBool);
	TestFalse(TEXT("Second duplicate does not consume twice"), OtherAbility->ValueBool);
	TestEqual(TEXT("Request cleared"), Request->ValueInt32, INDEX_NONE);
	TestNotNull(TEXT("Valid common request enables fast path"), Duplicates.ResolveSharedRequestNotify(Store, 0));
	Ability->ValueBool = false;
	Duplicates.ExecuteSystem(0, Stores);
	TestFalse(TEXT("Idle request causes no ability"), Ability->ValueBool);

	InitNotify(Second, Stores, 7, TAG_NotifyTestOtherRequest, TAG_NotifyTestOtherAbility);
	OtherRequest->ValueInt32 = 7;
	Duplicates.ExecuteSystem(0, Stores);
	TestTrue(TEXT("Different request buffer still executes"), OtherAbility->ValueBool);
	TestNull(TEXT("Different buffers disable fast path"), Duplicates.ResolveSharedRequestNotify(Store, 0));

	InitNotify(First, Stores, 7, TAG_NotifyTestRequest, TAG_NotifyTestAbility, false);
	FNotifyTestDispatcher Failed;
	Failed.Add(First);
	TestNull(TEXT("Failed initialization disables fast path"), Failed.ResolveSharedRequestNotify(Store, 0));
	Failed.ExecuteSystem(0, Stores);
	TestFalse(TEXT("Failed initialization safely skips ability"), Ability->ValueBool);

	InitNotify(First, Stores, 7, TAG_NotifyTestRequest, FGameplayTag());
	TestNull(TEXT("Valid request tag but failed output tag disables fast path"), Failed.ResolveSharedRequestNotify(Store, 0));
	Failed.ExecuteSystem(0, Stores);
	First.InitializeAs<FFluxPrimeCrowdsNotifyAbility>();
	TestNull(TEXT("Uninitialized fragment has no request array to index"), Failed.ResolveSharedRequestNotify(Store, 0));
	return true;
}
#endif
