#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Components/InstancedStaticMeshComponent.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsIdentity.h"
#include "Domains/Crowds/Modules/FluxPrimeCrowdsDataModule.h"
#include "Framework/Core/FluxPrimeBaseDataModule.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "NativeGameplayTags.h"
#include "UObject/UnrealType.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_DataRowLocation, "FluxPrime.Tests.DataRow.Location");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_DataRowRotation, "FluxPrime.Tests.DataRow.Rotation");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_DataRowId, "FluxPrime.Tests.DataRow.Id");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_DataRowSize, "FluxPrime.Tests.DataRow.Size");

namespace
{
FFluxPrimeCrowdsDataModule* MakeDataModule(UFluxPrimeDataStoreIdentity* Registry)
{
	FFluxPrimeCrowdsDataModule* Module = new FFluxPrimeCrowdsDataModule();
	auto* RegistryProperty = FindFProperty<FObjectProperty>(
		FFluxPrimeBaseDataModule::StaticStruct(), TEXT("DataStoreIdentity"));
	RegistryProperty->SetObjectPropertyValue_InContainer(Module, Registry);

	Module->DataTagConfig.EntityDataLocationConfig.EntityDataTag = TAG_DataRowLocation;
	Module->DataTagConfig.EntityDataLocationConfig.ExpectedStructType = TBaseStructure<FVector>::Get();
	Module->DataTagConfig.EntityDataRotationConfig.EntityDataTag = TAG_DataRowRotation;
	Module->DataTagConfig.EntityDataRotationConfig.ExpectedStructType = TBaseStructure<FRotator>::Get();
	Module->DataTagConfig.EntityDataIdConfig.EntityDataTag = TAG_DataRowId;
	Module->DataTagConfig.EntityDataIdConfig.ExpectedStructType = FFluxPrimeWrapperInt16::StaticStruct();
	Module->DataTagConfig.EntityDataSizeConfig.EntityDataTag = TAG_DataRowSize;
	Module->DataTagConfig.EntityDataSizeConfig.ExpectedStructType = FFluxPrimeWrapperInt16::StaticStruct();
	return Module;
}

void AddRegistryEntry(UFluxPrimeDataStoreIdentity* Registry, FGameplayTag Tag, UScriptStruct* Type)
{
	FFluxPrimeDataConfigEditor Entry;
	Entry.DataTag = Tag;
	Entry.StructType = Type;
	Entry.AvailableOnLocalClient = true;
	Registry->DataConfigEditors.Add(Entry);
}

void AddCatalogEntry(TArray<FFluxPrimeArchetypeEntityCatalog>& Catalog,
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>& Components,
	UObject* Outer, FName Name, int32 Size, int32 Pool)
{
	UFluxPrimeCrowdsIdentity* Identity = NewObject<UFluxPrimeCrowdsIdentity>(Outer);
	Identity->Identity = Name;
	Identity->Size = Size;

	FFluxPrimeArchetypeEntityCatalog Entry;
	Entry.EntityIdentity = Identity;
	Entry.TotalEntityOnPool = Pool;
	Catalog.Add(Entry);
	Components.Add(NewObject<UInstancedStaticMeshComponent>(Outer));
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeCrowdsDataRowTest, "FluxPrime.Critical.CrowdsDataRows",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeCrowdsDataRowTest::RunTest(const FString& Parameters)
{
	UObject* Outer = GetTransientPackage();

	UFluxPrimeDataStoreIdentity* Registry = NewObject<UFluxPrimeDataStoreIdentity>(Outer);
	AddRegistryEntry(Registry, TAG_DataRowLocation, TBaseStructure<FVector>::Get());
	AddRegistryEntry(Registry, TAG_DataRowRotation, TBaseStructure<FRotator>::Get());
	AddRegistryEntry(Registry, TAG_DataRowId, FFluxPrimeWrapperInt16::StaticStruct());
	AddRegistryEntry(Registry, TAG_DataRowSize, FFluxPrimeWrapperInt16::StaticStruct());

	TArray<FFluxPrimeArchetypeEntityCatalog> Catalog;
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> Components;
	AddCatalogEntry(Catalog, Components, Outer, TEXT("Small"), 3, 1);
	AddCatalogEntry(Catalog, Components, Outer, TEXT("Large"), 7, 3);

	TArray<FFluxPrimeArchetypeDataStore> Stores;
	FFluxPrimeCrowdsDataModuleContext Context;
	Context.crowdsComponents = &Components;
	Context.entityCatalog = &Catalog;
	Context.dataStores = &Stores;
	Context.hasAuthority = true;

	FFluxPrimeCrowdsDataModule* Module = MakeDataModule(Registry);
	Module->Initialized(Context);
	TestTrue(TEXT("Valid data rows initialize"), Module->IsAllDataValid);
	TestEqual(TEXT("One store per archetype"), Stores.Num(), 2);

	for (int32 Type = 0; Type < Catalog.Num(); ++Type)
	{
		const int32 Pool = Catalog[Type].TotalEntityOnPool;
		auto* Locations = reinterpret_cast<FVector*>(Stores[Type].GetRawBufferData(
			Stores[Type].GetDataID(TAG_DataRowLocation)));
		auto* Rotations = reinterpret_cast<FRotator*>(Stores[Type].GetRawBufferData(
			Stores[Type].GetDataID(TAG_DataRowRotation)));
		auto* Ids = reinterpret_cast<FFluxPrimeWrapperInt16*>(Stores[Type].GetRawBufferData(
			Stores[Type].GetDataID(TAG_DataRowId)));
		auto* Sizes = reinterpret_cast<FFluxPrimeWrapperInt16*>(Stores[Type].GetRawBufferData(
			Stores[Type].GetDataID(TAG_DataRowSize)));

		TestEqual(TEXT("Pool size is preserved"), Stores[Type].GetTotalEntityCount(), Pool);
		TestEqual(TEXT("Pool starts empty"), Stores[Type].GetActiveEntityCount(), 0);
		TestEqual(TEXT("Component owns one instance per row"), Components[Type]->GetInstanceCount(), Pool);

		for (int32 Row = 0; Row < Pool; ++Row)
		{
			TestTrue(TEXT("Location row is initialized"), Locations[Row].Equals(FVector::ZeroVector));
			TestTrue(TEXT("Rotation row is initialized"), Rotations[Row].Equals(FRotator::ZeroRotator));
			TestEqual(TEXT("Instance id follows the row"), Ids[Row].ValueInt16, Row);
			TestEqual(TEXT("Identity size follows the row"), Sizes[Row].ValueInt16,
				static_cast<UFluxPrimeCrowdsIdentity*>(Catalog[Type].EntityIdentity.Get())->Size);
		}
	}

	delete Module;

	// The id buffer is still Int16 until REF-028 widens it; reject a pool that would truncate.
	UFluxPrimeDataStoreIdentity* OverflowRegistry = NewObject<UFluxPrimeDataStoreIdentity>(Outer);
	AddRegistryEntry(OverflowRegistry, TAG_DataRowLocation, TBaseStructure<FVector>::Get());
	AddRegistryEntry(OverflowRegistry, TAG_DataRowRotation, TBaseStructure<FRotator>::Get());
	AddRegistryEntry(OverflowRegistry, TAG_DataRowId, FFluxPrimeWrapperInt16::StaticStruct());
	AddRegistryEntry(OverflowRegistry, TAG_DataRowSize, FFluxPrimeWrapperInt16::StaticStruct());

	TArray<FFluxPrimeArchetypeEntityCatalog> OverflowCatalog;
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> OverflowComponents;
	AddCatalogEntry(OverflowCatalog, OverflowComponents, Outer, TEXT("Overflow"), 1,
		TNumericLimits<int16>::Max() + 1);

	TArray<FFluxPrimeArchetypeDataStore> OverflowStores;
	FFluxPrimeCrowdsDataModuleContext OverflowContext;
	OverflowContext.crowdsComponents = &OverflowComponents;
	OverflowContext.entityCatalog = &OverflowCatalog;
	OverflowContext.dataStores = &OverflowStores;
	OverflowContext.hasAuthority = true;

	FFluxPrimeCrowdsDataModule* OverflowModule = MakeDataModule(OverflowRegistry);
	AddExpectedError(TEXT("exceeds the Int16 instance-id range"), EAutomationExpectedErrorFlags::Contains, 1);
	OverflowModule->Initialized(OverflowContext);
	TestFalse(TEXT("A pool that truncates the id buffer is rejected"), OverflowModule->IsAllDataValid);
	TestEqual(TEXT("Rejected pool adds no instances"), OverflowComponents[0]->GetInstanceCount(), 0);
	TestEqual(TEXT("Rejected pool leaves stores untouched"), OverflowStores.Num(), 0);

	delete OverflowModule;
	return true;
}
#endif
