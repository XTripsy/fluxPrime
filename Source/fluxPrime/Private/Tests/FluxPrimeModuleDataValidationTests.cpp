#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Framework/Core/FluxPrimeBaseModule.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "NativeGameplayTags.h"
#include "Domains/Crowds/Modules/FluxPrimeCrowdsDataModule.h"
#include "UObject/UnrealType.h"
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ModuleValidation, "FluxPrime.Tests.ModuleValidation");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ModuleRotation, "FluxPrime.Tests.ModuleValidation.Rotation");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ModuleId, "FluxPrime.Tests.ModuleValidation.Id");
namespace
{
struct FModuleValidationFixture : FFluxPrimeBaseModule
{
	using FFluxPrimeBaseModule::TryInitializedData;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeModuleDataValidationTest, "FluxPrime.High.ModuleDataValidation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFluxPrimeModuleDataValidationTest::RunTest(const FString& Parameters)
{
	FFluxPrimeArchetypeDataStore store;
	store.AddBuffer(TAG_ModuleValidation, FFluxPrimeWrapperInt16::StaticStruct(), sizeof(FFluxPrimeWrapperInt16), 1);
	FFluxPrimeDataConfig config;
	config.EntityDataTag = TAG_ModuleValidation;
	config.ExpectedStructType = FFluxPrimeWrapperInt16::StaticStruct();
	FModuleValidationFixture module;
	int16 id = INDEX_NONE;
	module.TryInitializedData(store, config, id);
	TestTrue(TEXT("Valid type resolves"), module.IsAllDataValid && id == 0);
	config.ExpectedStructType = FFluxPrimeWrapperInt8::StaticStruct();
	AddExpectedError(TEXT("owning module"), EAutomationExpectedErrorFlags::Contains, 1);
	module.TryInitializedData(store, config, id);
	TestFalse(TEXT("Wrong type invalidates module"), module.IsAllDataValid);
	TestEqual(TEXT("Failure clears cached index"), id, static_cast<int16>(INDEX_NONE));
	module.IsAllDataValid = true;
	config.ExpectedStructType = nullptr;
	AddExpectedError(TEXT("owning module"), EAutomationExpectedErrorFlags::Contains, 1);
	module.TryInitializedData(store, config, id);
	TestFalse(TEXT("Null expected type rejects resolution"), module.IsAllDataValid);

	FFluxPrimeCrowdsDataModule dataModule;
	UFluxPrimeDataStoreIdentity* registry = NewObject<UFluxPrimeDataStoreIdentity>();
	auto* registryProperty = FindFProperty<FObjectProperty>(FFluxPrimeBaseDataModule::StaticStruct(), TEXT("DataStoreIdentity"));
	if (!TestNotNull(TEXT("Registry property"), registryProperty)) return false;
	registryProperty->SetObjectPropertyValue_InContainer(&dataModule, registry);
	auto configure = [&](FFluxPrimeDataConfig& field, UScriptStruct* type)
	{
		field.EntityDataTag = TAG_ModuleValidation;
		field.ExpectedStructType = type;
	};
	configure(dataModule.DataTagConfig.EntityDataLocationConfig, TBaseStructure<FVector>::Get());
	configure(dataModule.DataTagConfig.EntityDataRotationConfig, TBaseStructure<FRotator>::Get());
	configure(dataModule.DataTagConfig.EntityDataIdConfig, FFluxPrimeWrapperInt16::StaticStruct());
	configure(dataModule.DataTagConfig.EntityDataSizeConfig, FFluxPrimeWrapperInt16::StaticStruct());
	TArray<FFluxPrimeArchetypeDataStore> stores;
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> components{NewObject<UInstancedStaticMeshComponent>()};
	TArray<FFluxPrimeArchetypeEntityCatalog> catalog;
	catalog.SetNum(1);
	catalog[0].EntityIdentity = NewObject<UFluxPrimeCrowdsIdentity>();
	catalog[0].TotalEntityOnPool = 2;
	FFluxPrimeCrowdsDataModuleContext context;
	context.dataStores = &stores;
	context.crowdsComponents = &components;
	context.entityCatalog = &catalog;
	context.hasAuthority = true;
	AddExpectedError(TEXT("owning module"), EAutomationExpectedErrorFlags::Contains, 1);
	dataModule.Initialized(context);
	TestFalse(TEXT("Missing registry entries invalidate data module"), dataModule.IsAllDataValid);
	TestEqual(TEXT("Invalid initialization adds no mesh instances"), components[0]->GetInstanceCount(), 0);
	dataModule.DataTagConfig.EntityDataRotationConfig.EntityDataTag = TAG_ModuleRotation;
	dataModule.DataTagConfig.EntityDataIdConfig.EntityDataTag = TAG_ModuleId;
	dataModule.DataTagConfig.EntityDataSizeConfig.EntityDataTag = TAG_ModuleId;
	for (const FFluxPrimeDataConfig* field : {&dataModule.DataTagConfig.EntityDataLocationConfig,
		&dataModule.DataTagConfig.EntityDataRotationConfig, &dataModule.DataTagConfig.EntityDataIdConfig})
	{
		FFluxPrimeDataConfigEditor entry;
		entry.DataTag = field->EntityDataTag;
		entry.StructType = field->ExpectedStructType;
		entry.AvailableOnLocalClient = true;
		registry->DataConfigEditors.Add(entry);
	}
	dataModule.Initialized(context);
	TestTrue(TEXT("Data module recovers after fixing configuration"), dataModule.IsAllDataValid);
	TestEqual(TEXT("Pool instances created"), components[0]->GetInstanceCount(), 2);
	dataModule.Initialized(context);
	TestEqual(TEXT("Reinitialization replaces instead of appending instances"), components[0]->GetInstanceCount(), 2);
	TestEqual(TEXT("Cached arrays do not accumulate"), dataModule.EntityDataLocationID.Num(), 1);
	auto* original = stores[0].GetRawBufferData(dataModule.EntityDataLocationID[0]);
	reinterpret_cast<FVector*>(original)[0] = FVector(123, 456, 789);
	dataModule.DataTagConfig.EntityDataLocationConfig.ExpectedStructType = FFluxPrimeWrapperInt8::StaticStruct();
	AddExpectedError(TEXT("owning module"), EAutomationExpectedErrorFlags::Contains, 1);
	dataModule.Initialized(context);
	TestFalse(TEXT("Invalid reinitialization fails"), dataModule.IsAllDataValid);
	TestTrue(TEXT("Failed reinitialization keeps live buffers"), stores[0].GetRawBufferData(0) == original);
	TestTrue(TEXT("Failed reinitialization preserves contents"), reinterpret_cast<FVector*>(original)[0].Equals(FVector(123, 456, 789)));
	TestEqual(TEXT("Failed reinitialization preserves instances"), components[0]->GetInstanceCount(), 2);
	return true;
}
#endif
