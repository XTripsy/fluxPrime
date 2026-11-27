#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileTypes.h"
#include "Domains/Crowds/Systems/Ability/FluxPrimeCrowdsMeleAbility.h"
#include "Domains/Crowds/Systems/Ability/FluxPrimeCrowdsProjectileAbility.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeDebugSeparationTest, "FluxPrime.Debug.NoSystemDebugProperties",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeDebugSeparationTest::RunTest(const FString& Parameters)
{
	TestNull(TEXT("Melee ability no longer exposes a debug flag"),
		FFluxPrimeCrowdsMeleAbility::StaticStruct()->FindPropertyByName(TEXT("IsDebug")));
	TestNull(TEXT("Projectile ability no longer exposes a debug flag"),
		FFluxPrimeCrowdsProjectileAbility::StaticStruct()->FindPropertyByName(TEXT("IsDebug")));
	TestNull(TEXT("Projectile shot no longer carries a debug flag"),
		FFluxPrimeProjectileShot::StaticStruct()->FindPropertyByName(TEXT("bDebug")));
	return true;
}
#endif
