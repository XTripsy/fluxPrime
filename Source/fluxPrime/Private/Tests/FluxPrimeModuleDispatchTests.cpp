#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Domains/Crowds/Data/FluxPrimeCrowdsPayloadData.h"
#include "Domains/Crowds/Managers/FluxPrimeCrowdsManager.h"
#include "Domains/Crowds/Modules/FluxPrimeCrowdsSpawnerModule.h"
#include "StructUtils/InstancedStruct.h"

namespace
{
struct FModuleDispatchTestManager : UFluxPrimeCrowdsManager
{
	void SetFragments(TArray<FInstancedStruct>* Fragments) { ModuleFragments = Fragments; }
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeModuleDispatchTest, "FluxPrime.High.ModuleDispatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeModuleDispatchTest::RunTest(const FString& Parameters)
{
	UObject* outer = GetTransientPackage();
	FModuleDispatchTestManager* manager = NewObject<FModuleDispatchTestManager>(outer);

	TestNull(TEXT("Lookup before PreLoading is null"),
		manager->FindModule<FFluxPrimeCrowdsSpawnerModule>(0));

	TArray<FInstancedStruct> fragments;
	fragments.Add(FInstancedStruct::Make(FFluxPrimeCrowdsSpawnerModule()));
	manager->SetFragments(&fragments);

	TestNotNull(TEXT("Matching module type resolves"),
		manager->FindModule<FFluxPrimeCrowdsSpawnerModule>(0));
	TestNull(TEXT("Mismatched module type is rejected"),
		manager->FindModule<FFluxPrimeCrowdsSystemsModule>(0));
	TestNull(TEXT("INDEX_NONE index is rejected"),
		manager->FindModule<FFluxPrimeCrowdsSpawnerModule>(INDEX_NONE));
	TestNull(TEXT("Out-of-range index is rejected"),
		manager->FindModule<FFluxPrimeCrowdsSpawnerModule>(1));

	manager->SetFragments(nullptr);
	TestNull(TEXT("Lookup after missing configuration is null"),
		manager->FindModule<FFluxPrimeCrowdsSpawnerModule>(0));

	manager->RequestAction(FInstancedStruct::Make(FFluxPrimeCrowdsSpawnPayload()));
	TestNull(TEXT("Spawn request before PreLoading leaves no module resolved"),
		manager->FindModule<FFluxPrimeCrowdsSpawnerModule>(0));

	FFluxPrimeCrowdsSnapshotDataPayload snapshot;
	manager->OnActionChange(FInstancedStruct::Make(snapshot));
	TestNull(TEXT("Snapshot before PreLoading leaves no module resolved"),
		manager->FindModule<FFluxPrimeCrowdsSystemsModule>(0));

	return true;
}
#endif
