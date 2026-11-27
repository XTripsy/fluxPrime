#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Framework/Systems/Debug/FluxPrimeDebugPass.h"
#include "Framework/Configurations/FluxPrimeDebugConfiguration.h"
#include "Generics/Systems/Debug/FluxPrimeMovementDebugSystem.h"
#include "Generics/Systems/Debug/FluxPrimeIdentityDebugSystem.h"
#include "Generics/Systems/Debug/FluxPrimeHealthDebugSystem.h"
#include "Generics/Systems/Debug/FluxPrimeAnimationDebugSystem.h"
#include "Generics/Systems/Debug/FluxPrimeBoidsDebugSystem.h"
#include "Generics/Systems/Debug/FluxPrimeNavigationDebugSystem.h"
#include "Generics/Systems/Debug/FluxPrimeSpatialGridDebugSystem.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#include "NativeGameplayTags.h"
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ModularPosition, "FluxPrime.Tests.Modular.Position");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ModularInt16, "FluxPrime.Tests.Modular.Int16");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ModularInt32, "FluxPrime.Tests.Modular.Int32");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ModularInt8, "FluxPrime.Tests.Modular.Int8");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ModularFloat, "FluxPrime.Tests.Modular.Float");
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeModularTest, "FluxPrime.Debug.Modular", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFluxPrimeModularTest::RunTest(const FString& Parameters)
{
	TArray<FFluxPrimeArchetypeDataStore> Stores;
	Stores.SetNum(1);
	auto Add = [&](FGameplayTag Tag, UScriptStruct* Type) { Stores[0].AddBuffer(Tag, Type, Type->GetStructureSize(), 2); FFluxPrimeDataConfig C; C.EntityDataTag = Tag; C.ExpectedStructType = Type; return C; };
	const auto Position = Add(TAG_ModularPosition, TBaseStructure<FVector>::Get());
	const auto Int16 = Add(TAG_ModularInt16, FFluxPrimeWrapperInt16::StaticStruct());
	const auto Int32 = Add(TAG_ModularInt32, FFluxPrimeWrapperInt32::StaticStruct());
	const auto Int8 = Add(TAG_ModularInt8, FFluxPrimeWrapperInt8::StaticStruct());
	const auto Float = Add(TAG_ModularFloat, FFluxPrimeWrapperFloat::StaticStruct());
	Stores[0].SetActiveEntityCount(2);
	reinterpret_cast<FVector*>(Stores[0].GetRawBufferData(0))[0] = FVector(3,4,0);
	reinterpret_cast<FVector*>(Stores[0].GetRawBufferData(0))[1] = FVector(100,0,0);
	reinterpret_cast<FFluxPrimeWrapperInt16*>(Stores[0].GetRawBufferData(1))[0].ValueInt16 = 50;
	reinterpret_cast<FFluxPrimeWrapperInt32*>(Stores[0].GetRawBufferData(2))[0].ValueInt32 = 0;
	reinterpret_cast<FFluxPrimeWrapperInt32*>(Stores[0].GetRawBufferData(2))[1].ValueInt32 = 0;
	reinterpret_cast<FFluxPrimeWrapperInt8*>(Stores[0].GetRawBufferData(3))[0].ValueInt8 = 3;
	reinterpret_cast<FFluxPrimeWrapperFloat*>(Stores[0].GetRawBufferData(4))[0].ValueFloat = 150;
	TArray<FInstancedStruct> Systems;
	auto& Move = Systems.Add_GetRef(FInstancedStruct::Make<FFluxPrimeMovementDebugSystem>()).GetMutable<FFluxPrimeMovementDebugSystem>();
	Move.Location = Move.Velocity = Move.Waypoint = Position; Move.MaxSpeed = Float;
	auto& Identity = Systems.Add_GetRef(FInstancedStruct::Make<FFluxPrimeIdentityDebugSystem>()).GetMutable<FFluxPrimeIdentityDebugSystem>();
	Identity.Location = Position; Identity.StoredId = Int16;
	auto& Health = Systems.Add_GetRef(FInstancedStruct::Make<FFluxPrimeHealthDebugSystem>()).GetMutable<FFluxPrimeHealthDebugSystem>();
	Health.Location = Position; Health.Health = Int16;
	auto& Anim = Systems.Add_GetRef(FInstancedStruct::Make<FFluxPrimeAnimationDebugSystem>()).GetMutable<FFluxPrimeAnimationDebugSystem>();
	Anim.Location = Position; Anim.State = Int8;
	auto& Boids = Systems.Add_GetRef(FInstancedStruct::Make<FFluxPrimeBoidsDebugSystem>()).GetMutable<FFluxPrimeBoidsDebugSystem>();
	Boids.Location = Position; Boids.Size = Int16;
	auto& Nav = Systems.Add_GetRef(FInstancedStruct::Make<FFluxPrimeNavigationDebugSystem>()).GetMutable<FFluxPrimeNavigationDebugSystem>();
	Nav.CurrentLocation = Nav.TargetLocation = Position;
	auto& Grid = Systems.Add_GetRef(FInstancedStruct::Make<FFluxPrimeSpatialGridDebugSystem>()).GetMutable<FFluxPrimeSpatialGridDebugSystem>();
	Grid.Location = Position; Grid.CellId = Int32;
	TArray<TArray<uint8>> Before;
	for (int32 I = 0; I < 5; ++I) { auto& Bytes = Before.AddDefaulted_GetRef(); const int32 Size = (I == 0 ? sizeof(FVector) : I == 1 ? sizeof(FFluxPrimeWrapperInt16) : I == 2 ? sizeof(FFluxPrimeWrapperInt32) : I == 3 ? sizeof(FFluxPrimeWrapperInt8) : sizeof(FFluxPrimeWrapperFloat)) * 2; Bytes.Append(Stores[0].PeekRawBufferData(I), Size); }
	TArray<FFluxPrimeDebugRuntimeEntry> Entries;
	FluxPrimeBuildDebugRuntime(Systems, Entries);
	TestEqual(TEXT("All seven registered"), Entries.Num(), 7);
	TArray<FFluxPrimeDebugArchetypeMeta> Meta{{TEXT("CrowdA"),100}};
	FFluxPrimeDebugReadView View(Stores);
	FluxPrimeInitializeDebugSystems(View, Meta, Systems, Entries);
	FFluxPrimeDebugLimits Limits;
	FFluxPrimeDebugFrame Frame;
	auto Run = [&]() { FluxPrimeRunDebugPass(View, Meta, Position, Limits, 180, 24, FVector::ZeroVector, Systems, Entries, Frame); };
	Run();
	TestEqual(TEXT("Seven labels per row"), Frame.Labels.Num(), 14);
	if (Frame.Labels.Num() == 14)
	{
		TestEqual(TEXT("Movement"), Frame.Labels[0].Text, FString(TEXT("CrowdA #0 | v=5 | max=150 | wp=0")));
		TestEqual(TEXT("Identity"), Frame.Labels[1].Text, FString(TEXT("CrowdA | renderRow=0 | storedId=50")));
		TestEqual(TEXT("Health"), Frame.Labels[2].Text, FString(TEXT("Health #0 | hp=50 | alive")));
		TestEqual(TEXT("Animation optional values"), Frame.Labels[3].Text, FString(TEXT("Anim #0 | state=3 | anim=n/a | t=n/a | notify=n/a")));
		TestEqual(TEXT("Boids"), Frame.Labels[4].Text, FString(TEXT("Boids #0 | size=50 | radius=30.0")));
		TestEqual(TEXT("Navigation"), Frame.Labels[5].Text, FString(TEXT("Nav #0 | wp=0 | wait=0.0")));
		TestEqual(TEXT("Grid sampled occupancy"), Frame.Labels[6].Text, FString(TEXT("Grid #0 | Stored cell (post-manager)=0 | members=2 (sampled)")));
		for (int32 I = 0; I < 7; ++I) TestEqual(TEXT("Stack order"), Frame.Labels[I].Location.Z, double(180 + 24*I));
	}
	Entries[1].bRuntimeEnabled = false;
	Run();
	TestEqual(TEXT("Runtime disabled"), Frame.Labels.Num(), 12);
	if (Frame.Labels.Num() > 1) TestEqual(TEXT("No stacking gap"), Frame.Labels[1].Location.Z, 204.0);
	Entries[1].bRuntimeEnabled = true;
	Run(); TestEqual(TEXT("Runtime re-enabled"), Frame.Labels.Num(), 14);
	Limits.MaxLabels = 8;
	Run();
	if (Frame.Labels.Num() == 8) TestTrue(TEXT("All systems label nearest before farther row"), Frame.Labels[7].Text.Contains(TEXT("#1")));
	Systems[0].GetMutable<FFluxPrimeMovementDebugSystem>().MaxSpeed.ExpectedStructType = TBaseStructure<FVector>::Get();
	FluxPrimeInitializeDebugSystems(View, Meta, Systems, Entries);
	Limits.MaxLabels = 256; Run();
	TestEqual(TEXT("Invalid system fails locally"), Frame.Labels.Num(), 12);
	for (int32 I = 0; I < 5; ++I) TestTrue(TEXT("No entity writes"), FMemory::Memcmp(Before[I].GetData(), Stores[0].PeekRawBufferData(I), Before[I].Num()) == 0);
	return true;
}
#endif
