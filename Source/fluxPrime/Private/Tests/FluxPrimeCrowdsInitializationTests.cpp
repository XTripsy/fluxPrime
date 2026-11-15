#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Domains/Crowds/Configurations/ManagerConfiguration.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsIdentity.h"
#include "Domains/Crowds/Managers/FluxPrimeCrowdsManager.h"
#include "Framework/Data/FluxPrimeEntityCatalogIdentity.h"
#include "Framework/Data/FluxPrimeStateIdentity.h"

namespace
{
struct FCrowdsInitializationTestManager : UFluxPrimeCrowdsManager
{
	using UFluxPrimeCrowdsManager::ValidateAndResolveConfiguration;

	void Configure(UManagerConfiguration* configuration, UFluxPrimeEntityCatalogIdentity* catalog)
	{
		ManagerConfiguration = configuration;
		EntityCatalogIdentity = catalog;
	}
};

UFluxPrimeCrowdsIdentity* MakeIdentity(UObject* outer, FName name, bool withState = true)
{
	UFluxPrimeCrowdsIdentity* identity = NewObject<UFluxPrimeCrowdsIdentity>(outer);
	identity->Identity = name;

	if (withState)
	{
		identity->StateIdentity = NewObject<UFluxPrimeStateIdentity>(outer);
	}

	return identity;
}

void AddCatalogEntry(UFluxPrimeEntityCatalogIdentity* catalog, UFluxPrimeBaseIdentity* identity)
{
	FFluxPrimeArchetypeEntityCatalog entry;
	entry.EntityIdentity = identity;
	entry.TotalEntityOnPool = 10;
	catalog->ArchetypeEntityCatalog.Add(entry);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeCrowdsInitializationTest, "FluxPrime.High.CrowdsInitialization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeCrowdsInitializationTest::RunTest(const FString& Parameters)
{
	UObject* outer = GetTransientPackage();

	UManagerConfiguration* configuration = NewObject<UManagerConfiguration>(outer);
	UFluxPrimeEntityCatalogIdentity* catalog = NewObject<UFluxPrimeEntityCatalogIdentity>(outer);

	FCrowdsInitializationTestManager* missingConfiguration = NewObject<FCrowdsInitializationTestManager>(outer);
	missingConfiguration->Configure(nullptr, catalog);
	AddExpectedError(TEXT("ManagerConfiguration is not set"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("Missing manager configuration aborts"), missingConfiguration->ValidateAndResolveConfiguration());
	TestEqual(TEXT("No identities resolve without configuration"), missingConfiguration->GetResolvedCrowdIdentities().Num(), 0);

	FCrowdsInitializationTestManager* missingCatalog = NewObject<FCrowdsInitializationTestManager>(outer);
	missingCatalog->Configure(configuration, nullptr);
	AddExpectedError(TEXT("EntityCatalogIdentity is not set"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("Missing entity catalog aborts"), missingCatalog->ValidateAndResolveConfiguration());

	FCrowdsInitializationTestManager* emptyCatalog = NewObject<FCrowdsInitializationTestManager>(outer);
	emptyCatalog->Configure(configuration, catalog);
	AddExpectedError(TEXT("the entity catalog is empty"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("Empty catalog aborts"), emptyCatalog->ValidateAndResolveConfiguration());

	AddCatalogEntry(catalog, nullptr);
	AddExpectedError(TEXT("catalog index 0 holds null"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("Null catalog entry aborts"), emptyCatalog->ValidateAndResolveConfiguration());
	TestEqual(TEXT("Failed validation clears resolved identities"), emptyCatalog->GetResolvedCrowdIdentities().Num(), 0);

	catalog->ArchetypeEntityCatalog.Reset();
	AddCatalogEntry(catalog, MakeIdentity(outer, NAME_None));
	AddExpectedError(TEXT("has an empty Identity name"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("Empty identity name aborts"), emptyCatalog->ValidateAndResolveConfiguration());

	catalog->ArchetypeEntityCatalog.Reset();
	AddCatalogEntry(catalog, MakeIdentity(outer, TEXT("Archetype"), false));
	AddExpectedError(TEXT("has no StateIdentity"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("Missing state identity aborts"), emptyCatalog->ValidateAndResolveConfiguration());

	catalog->ArchetypeEntityCatalog.Reset();
	AddCatalogEntry(catalog, MakeIdentity(outer, TEXT("Archetype")));
	AddCatalogEntry(catalog, MakeIdentity(outer, TEXT("Archetype")));
	AddExpectedError(TEXT("repeats identity name"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("Repeated identity name aborts"), emptyCatalog->ValidateAndResolveConfiguration());

	catalog->ArchetypeEntityCatalog.Reset();
	UFluxPrimeCrowdsIdentity* first = MakeIdentity(outer, TEXT("First"));
	UFluxPrimeCrowdsIdentity* second = MakeIdentity(outer, TEXT("Second"));
	AddCatalogEntry(catalog, first);
	AddCatalogEntry(catalog, second);

	TestTrue(TEXT("Valid catalog resolves"), emptyCatalog->ValidateAndResolveConfiguration());
	TestEqual(TEXT("Every catalog entry resolves"), emptyCatalog->GetResolvedCrowdIdentities().Num(), 2);
	TestTrue(TEXT("Resolution preserves catalog order"), emptyCatalog->GetResolvedCrowdIdentities()[0].Get() == first);
	TestTrue(TEXT("Resolution preserves trailing order"), emptyCatalog->GetResolvedCrowdIdentities()[1].Get() == second);

	return true;
}
#endif
