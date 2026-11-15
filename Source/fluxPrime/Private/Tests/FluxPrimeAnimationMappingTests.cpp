#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Generics/Data/FluxPrimeAnimationIdentity.h"
#include "Generics/Systems/Animation/FluxPrimeAnimationSystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeAnimationMappingTest, "FluxPrime.High.AnimationMapping",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeAnimationMappingTest::RunTest(const FString& Parameters)
{
	UObject* outer = GetTransientPackage();

	FFluxPrimeAnimationSystem system;
	TArray<TObjectPtr<UFluxPrimeAnimationData>> resolved;

	TestFalse(TEXT("Empty catalog cannot resolve a mapping"), system.InitializeAnimationMapping(resolved));

	resolved.Add(nullptr);
	AddExpectedError(TEXT("no animation asset is resolved"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("A missing archetype asset aborts mapping"), system.InitializeAnimationMapping(resolved));

	resolved.Reset();
	resolved.Add(NewObject<UFluxPrimeAnimationData>(outer));
	TestTrue(TEXT("A fully resolved archetype set builds a mapping"), system.InitializeAnimationMapping(resolved));

	return true;
}
#endif
