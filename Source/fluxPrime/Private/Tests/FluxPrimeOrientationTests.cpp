#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Domains/Crowds/Configurations/SystemsConfiguration.h"
#include "UObject/UnrealType.h"
#include "Generics/Systems/Animation/FluxPrimeAnimationSystem.h"
#include "Generics/Systems/Navigation/FluxPrimeNavigationSystem.h"
#include "Generics/Systems/FluxPrimeMovementSystem.h"
#include "Generics/Systems/FluxPrimeVisualInterpolationSystem.h"
#include "Domains/Crowds/Systems/FluxPrimeCrowdsAbilitySystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeOrientationTest, "FluxPrime.Medium.SharedOrientation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFluxPrimeOrientationTest::RunTest(const FString& Parameters)
{
	USystemsConfiguration* Config = NewObject<USystemsConfiguration>();
	FStructProperty* Property = FindFProperty<FStructProperty>(Config->GetClass(), TEXT("MeshOrientation"));
	if (!TestNotNull(TEXT("One shared orientation source"), Property)) return false;
	FFloatProperty* Yaw = FindFProperty<FFloatProperty>(Property->Struct, TEXT("ForwardYawOffsetDegrees"));
	if (!TestNotNull(TEXT("Yaw is configurable"), Yaw)) return false;
	void* Value = Property->ContainerPtrToValuePtr<void>(Config);
	TestEqual(TEXT("Legacy yaw default"), Yaw->GetPropertyValue_InContainer(Value), -85.0f);
	FFluxPrimeMovementSystem Movement;
	FFluxPrimeVisualInterpolationSystem Interpolation;
	FFluxPrimeCrowdsAbilitySystem Ability;
	Movement.MeshOrientation = &Config->MeshOrientation;
	Interpolation.MeshOrientation = &Config->MeshOrientation;
	Ability.MeshOrientation = &Config->MeshOrientation;
	for (float Offset : {-85.0f, 0.0f, 90.0f})
	{
		Config->MeshOrientation.ForwardYawOffsetDegrees = Offset;
		TestEqual(TEXT("Movement orientation"), Movement.MeshOrientation->ToMeshYaw(20.0f), 20.0f + Offset);
		TestEqual(TEXT("Interpolation agrees"), Interpolation.MeshOrientation->ToMeshYaw(20.0f), 20.0f + Offset);
		TestEqual(TEXT("Ability uses inverse"), Ability.MeshOrientation->ToTravelYaw(20.0f + Offset), 20.0f);
	}
	auto CheckDefault = [this](UScriptStruct* Type, const void* Object, const TCHAR* Name, float Expected)
	{
		FFloatProperty* Field = FindFProperty<FFloatProperty>(Type, Name);
		if (TestNotNull(Name, Field)) TestEqual(Name, Field->GetPropertyValue_InContainer(Object), Expected);
	};
	CheckDefault(Movement.StaticStruct(), &Movement, TEXT("SteeringResponsiveness"), 8.0f);
	CheckDefault(Movement.StaticStruct(), &Movement, TEXT("SlowdownAngleDegrees"), 120.0f);
	CheckDefault(Movement.StaticStruct(), &Movement, TEXT("MinMoveFactor"), 0.25f);
	CheckDefault(Movement.StaticStruct(), &Movement, TEXT("VelocityOverspeedFactor"), 1.2f);
	CheckDefault(Movement.StaticStruct(), &Movement, TEXT("AvoidanceWeight"), 0.5f);
	CheckDefault(Movement.StaticStruct(), &Movement, TEXT("HoldAvoidanceWeight"), 0.5f);
	FFluxPrimeAnimationSystem Animation;
	CheckDefault(Animation.StaticStruct(), &Animation, TEXT("SourceFrameRate"), 30.0f);
	FFluxPrimeNavigationSystem Navigation;
	CheckDefault(Navigation.StaticStruct(), &Navigation, TEXT("ArrivalRadius"), 60.0f);
	CheckDefault(Navigation.StaticStruct(), &Navigation, TEXT("ArrivalSizeMultiplier"), 1.5f);
	CheckDefault(Navigation.StaticStruct(), &Navigation, TEXT("NavMeshProjectionSizeMultiplier"), 1.5f);
	FFluxPrimeCrowdsMeleAbility Melee;
	CheckDefault(Melee.StaticStruct(), &Melee, TEXT("TraceForwardDistance"), 200.0f);
	CheckDefault(Melee.StaticStruct(), &Melee, TEXT("TraceSphereRadius"), 50.0f);
	CheckDefault(Melee.StaticStruct(), &Melee, TEXT("TraceHeightOffset"), 75.0f);
	TestEqual(TEXT("Melee channel default"), Melee.TraceChannel.GetValue(), ECC_Pawn);
	return true;
}
#endif
