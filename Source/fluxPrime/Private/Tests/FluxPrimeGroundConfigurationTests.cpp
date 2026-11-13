#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Generics/Systems/FluxPrimeGroundHeightSystem.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeGroundConfigurationTest, "FluxPrime.Medium.GroundConfiguration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeGroundConfigurationTest::RunTest(const FString& Parameters)
{
	FFluxPrimeGroundHeightSystem System;
	FByteProperty* Channel = FindFProperty<FByteProperty>(System.StaticStruct(), TEXT("TraceChannel"));
	if (!TestNotNull(TEXT("Ground channel is configurable"), Channel)) return false;
	TestEqual(TEXT("Existing channel default is preserved"), Channel->GetPropertyValue_InContainer(&System), uint8(ECC_GameTraceChannel1));
	Channel->SetPropertyValue_InContainer(&System, uint8(ECC_Visibility));
	TestEqual(TEXT("Channel override is retained"), Channel->GetPropertyValue_InContainer(&System), uint8(ECC_Visibility));
	return true;
}
#endif
