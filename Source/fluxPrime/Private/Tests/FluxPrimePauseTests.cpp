#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Tests/FluxPrimePauseTestManager.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/PlayerState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimePauseDispatchTest, "FluxPrime.Pause.Dispatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimePauseDispatchTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	AActor* Owner = World->SpawnActor<AActor>();
	APlayerState* Pauser = World->SpawnActor<APlayerState>();
	UFluxPrimePauseTestManager* Manager = NewObject<UFluxPrimePauseTestManager>(Owner);
	Manager->RegisterComponent();
	FInstancedStruct Storage = FInstancedStruct::Make<FFluxPrimePauseCounterModule>();
	Manager->InstallCounter(Storage, [](FInstancedStruct& Data, float, TArray<FFluxPrimeArchetypeDataStore>&)
	{
		++Data.GetMutable<FFluxPrimePauseCounterModule>().Count;
	});
	Manager->Dispatch();
	TestEqual(TEXT("Running dispatch executes"), Storage.Get<FFluxPrimePauseCounterModule>().Count, 1);
	World->GetWorldSettings()->SetPauserPlayerState(Pauser);
	TestTrue(TEXT("Fixture world is paused"), World->IsPaused());
	Manager->Dispatch();
	Manager->Dispatch();
	TestEqual(TEXT("Direct dispatch cannot bypass world pause"), Storage.Get<FFluxPrimePauseCounterModule>().Count, 1);
	World->GetWorldSettings()->SetPauserPlayerState(nullptr);
	Manager->Dispatch();
	TestEqual(TEXT("Resume dispatch executes once without catch-up"), Storage.Get<FFluxPrimePauseCounterModule>().Count, 2);
	World->DestroyWorld(false);
	return true;
}

#include "Framework/Core/FluxPrimePauseWorldSubsystem.h"
#include "NiagaraComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimePausePresentationTest, "FluxPrime.Pause.Presentation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFluxPrimePausePresentationTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	UFluxPrimePauseWorldSubsystem* Service = World->GetSubsystem<UFluxPrimePauseWorldSubsystem>();
	if (!TestNotNull(TEXT("World owns pause service"), Service)) { World->DestroyWorld(false); return false; }
	UNiagaraComponent* Effect = NewObject<UNiagaraComponent>(World);
	Effect->SetAutoActivate(false);
	Effect->RegisterComponentWithWorld(World);
	Service->RegisterNiagaraComponent(Effect);
	APlayerState* Pauser = World->SpawnActor<APlayerState>();
	World->GetWorldSettings()->SetPauserPlayerState(Pauser);
	FWorldDelegates::OnWorldTickStart.Broadcast(World, LEVELTICK_All, 0.1f);
	TestTrue(TEXT("World pause freezes registered presentation"), Effect->IsPaused());
	Service->RegisterNiagaraComponent(Effect);
	World->GetWorldSettings()->SetPauserPlayerState(nullptr);
	FWorldDelegates::OnWorldTickStart.Broadcast(World, LEVELTICK_All, 0.1f);
	TestFalse(TEXT("Duplicate registration preserves resume ownership"), Effect->IsPaused());
	Effect->SetPaused(true);
	World->GetWorldSettings()->SetPauserPlayerState(Pauser);
	FWorldDelegates::OnWorldTickStart.Broadcast(World, LEVELTICK_All, 0.1f);
	World->GetWorldSettings()->SetPauserPlayerState(nullptr);
	FWorldDelegates::OnWorldTickStart.Broadcast(World, LEVELTICK_All, 0.1f);
	TestTrue(TEXT("Independent pause survives world resume"), Effect->IsPaused());
	Service->UnregisterNiagaraComponent(Effect);
	Effect->DestroyComponent();
	World->DestroyWorld(false);
	return true;
}

#include "Domains/Crowds/FluxPrimeCrowdsProjectileBatchSubsystem.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimePauseSubmissionTest, "FluxPrime.Pause.Submission",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFluxPrimePauseSubmissionTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	World->GetWorldSettings()->SetPauserPlayerState(World->SpawnActor<APlayerState>());
	auto* Batch = World->GetSubsystem<UFluxPrimeCrowdsProjectileBatchSubsystem>();
	TestFalse(TEXT("Paused shot rejected"), Batch->SubmitShot({}, {}));
	TestEqual(TEXT("Pause rejects before asset validation"), static_cast<int32>(Batch->GetLastRejection()),
		static_cast<int32>(EFluxPrimeProjectileRejection::Paused));
	World->DestroyWorld(false);
	return true;
}

#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileDamageHandler.h"
#include "Tests/FluxPrimeCrowdsProjectileTestTarget.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimePauseImpactTest, "FluxPrime.Pause.AcceptedImpact",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFluxPrimePauseImpactTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	auto* Target = World->SpawnActor<AFluxPrimeCrowdsProjectileTestTarget>();
	auto* Handler = NewObject<UFluxPrimeCrowdsProjectileDamageHandler>();
	Handler->Initialize(World, false);
	TestTrue(TEXT("Reserve first result"), Handler->ReserveShot(1, 10, 1));
	TestFalse(TEXT("Capacity rejects before publication"), Handler->ReserveShot(2, 10, 1));
	World->GetWorldSettings()->SetPauserPlayerState(World->SpawnActor<APlayerState>());
	FWorldDelegates::OnWorldTickStart.Broadcast(World, LEVELTICK_All, 0.1f);
	FFluxPrimeProjectileParticleData Particle;
	Particle.Position = Target->GetActorLocation();
	Particle.Damage = 17;
	Particle.ShotId = 1;
	Handler->ReceiveProjectileParticles_Implementation({Particle}, nullptr, FVector::ZeroVector);
	Handler->ReceiveProjectileParticles_Implementation({Particle}, nullptr, FVector::ZeroVector);
	TestEqual(TEXT("Paused callback applies no damage"), Target->GetTotalDamage(), 0.0f);
	Target->SetActorLocation(FVector(1000, 0, 0));
	World->GetWorldSettings()->SetPauserPlayerState(nullptr);
	FWorldDelegates::OnWorldTickStart.Broadcast(World, LEVELTICK_All, 0.1f);
	TestEqual(TEXT("Resume applies resolved hit exactly once"), Target->GetTotalDamage(), 17.0f);
	TestTrue(TEXT("Delivered result frees capacity"), Handler->ReserveShot(2, 10, 1));
	World->TimeSeconds = 11;
	TestTrue(TEXT("Expired miss frees capacity"), Handler->ReserveShot(3, 10, 1));
	Handler->Shutdown();
	World->DestroyWorld(false);
	return true;
}

#include "Domains/Crowds/Niagara/FluxPrimeCrowdsNiagaraCallback.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimePauseLegacyImpactTest, "FluxPrime.Pause.LegacyImpact",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFluxPrimePauseLegacyImpactTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	auto* Target = World->SpawnActor<AFluxPrimeCrowdsProjectileTestTarget>();
	auto* Handler = NewObject<UFluxPrimeCrowdsNiagaraCallback>();
	FFluxPrimeCrowdsNiagaraCallbackContext Context;
	Context.world = World;
	Context.damage = 9;
	Handler->InitializedNiagaraCallback(Context);
	World->GetWorldSettings()->SetPauserPlayerState(World->SpawnActor<APlayerState>());
	FWorldDelegates::OnWorldTickStart.Broadcast(World, LEVELTICK_All, 0.1f);
	FBasicParticleData Particle;
	Particle.Position = Target->GetActorLocation();
	Handler->ReceiveParticleData_Implementation({Particle}, nullptr, FVector::ZeroVector);
	TestEqual(TEXT("Legacy callback cannot damage while paused"), Target->GetTotalDamage(), 0.0f);
	World->GetWorldSettings()->SetPauserPlayerState(nullptr);
	FWorldDelegates::OnWorldTickStart.Broadcast(World, LEVELTICK_All, 0.1f);
	TestEqual(TEXT("Legacy resolved impact survives pause"), Target->GetTotalDamage(), 9.0f);
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimePauseReentryTest, "FluxPrime.Pause.ReentrantResume",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFluxPrimePauseReentryTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	auto* Target = World->SpawnActor<AFluxPrimeCrowdsProjectileTestTarget>();
	auto* Pauser = World->SpawnActor<APlayerState>();
	auto* Handler = NewObject<UFluxPrimeCrowdsProjectileDamageHandler>();
	Handler->Initialize(World, false);
	Handler->ReserveShot(1, 10, 2);
	Handler->ReserveShot(2, 10, 2);
	World->GetWorldSettings()->SetPauserPlayerState(Pauser);
	FFluxPrimeProjectileParticleData Particle;
	Particle.Damage = 1;
	Particle.ShotId = 1;
	Handler->ReceiveProjectileParticles_Implementation({Particle}, nullptr, FVector::ZeroVector);
	Particle.ShotId = 2;
	Handler->ReceiveProjectileParticles_Implementation({Particle}, nullptr, FVector::ZeroVector);
	Target->OnTestDamage = [World, Pauser]() { World->GetWorldSettings()->SetPauserPlayerState(Pauser); };
	World->GetWorldSettings()->SetPauserPlayerState(nullptr);
	// No observer tick sampled pause entry.
	FWorldDelegates::OnWorldTickStart.Broadcast(World, LEVELTICK_All, 0.1f);
	TestEqual(TEXT("Unobserved pause drains, but reentrant pause stops at first result"), Target->GetDamageEventCount(), 1);
	Target->OnTestDamage = nullptr;
	World->GetWorldSettings()->SetPauserPlayerState(nullptr);
	FWorldDelegates::OnWorldTickStart.Broadcast(World, LEVELTICK_All, 0.1f);
	TestEqual(TEXT("Remaining result survives reentrant pause"), Target->GetDamageEventCount(), 2);
	Handler->Shutdown();
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimePauseObservationTest, "FluxPrime.Pause.UnobservedCycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFluxPrimePauseObservationTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	auto* Target = World->SpawnActor<AFluxPrimeCrowdsProjectileTestTarget>();
	auto* Pauser = World->SpawnActor<APlayerState>();
	auto* Handler = NewObject<UFluxPrimeCrowdsProjectileDamageHandler>();
	Handler->Initialize(World, false);
	Handler->ReserveShot(1, 10, 1);
	World->GetWorldSettings()->SetPauserPlayerState(Pauser);
	FWorldDelegates::OnWorldTickStart.Broadcast(World, LEVELTICK_All, 0.1f);
	FFluxPrimeProjectileParticleData Particle;
	Particle.Position = Target->GetActorLocation();
	Particle.Damage = 4;
	Particle.ShotId = 1;
	Handler->ReceiveProjectileParticles_Implementation({Particle}, nullptr, FVector::ZeroVector);
	// Resume without an observer tick sampling paused.
	World->GetWorldSettings()->SetPauserPlayerState(nullptr);
	// No observer tick at all: the supported simulation boundary must drain first.
	Handler->FlushPendingDamage();
	TestEqual(TEXT("Pending result survives a short pause cycle"), Target->GetDamageEventCount(), 1);
	Handler->Shutdown();
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimePauseStaleTargetTest, "FluxPrime.Pause.StaleTarget",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFluxPrimePauseStaleTargetTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	auto* Target = World->SpawnActor<AFluxPrimeCrowdsProjectileTestTarget>();
	auto* Pauser = World->SpawnActor<APlayerState>();
	auto* Handler = NewObject<UFluxPrimeCrowdsProjectileDamageHandler>();
	Handler->Initialize(World, false);
	Handler->ReserveShot(1, 10, 1);
	World->GetWorldSettings()->SetPauserPlayerState(Pauser);
	FWorldDelegates::OnWorldTickStart.Broadcast(World, LEVELTICK_All, 0.1f);
	FFluxPrimeProjectileParticleData Particle;
	Particle.Position = Target->GetActorLocation();
	Particle.Damage = 7;
	Particle.ShotId = 1;
	Handler->ReceiveProjectileParticles_Implementation({Particle}, nullptr, FVector::ZeroVector);
	Target->Destroy();
	const uint64 Before = FFluxPrimeCrowdsProjectileDiagnostics::Get().DamageApplications;
	World->GetWorldSettings()->SetPauserPlayerState(nullptr);
	Handler->FlushPendingDamage();
	TestEqual(TEXT("Destroyed target cannot receive deferred damage"), FFluxPrimeCrowdsProjectileDiagnostics::Get().DamageApplications, Before);
	Handler->Shutdown();
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimePauseIsolationTest, "FluxPrime.Pause.WorldIsolation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFluxPrimePauseIsolationTest::RunTest(const FString& Parameters)
{
	UWorld* A = UWorld::CreateWorld(EWorldType::Game, false);
	UWorld* B = UWorld::CreateWorld(EWorldType::Game, false);
	B->GetWorldSettings()->SetPauserPlayerState(B->SpawnActor<APlayerState>());
	UNiagaraComponent* EffectA = NewObject<UNiagaraComponent>(A);
	EffectA->RegisterComponentWithWorld(A);
	A->GetSubsystem<UFluxPrimePauseWorldSubsystem>()->RegisterNiagaraComponent(EffectA);
	FWorldDelegates::OnWorldTickStart.Broadcast(B, LEVELTICK_All, 0.1f);
	TestFalse(TEXT("Another paused world cannot freeze this presentation"), EffectA->IsPaused());
	FWorldDelegates::OnWorldTickStart.Broadcast(A, LEVELTICK_All, 0.1f);
	EffectA->DestroyComponent();
	TestFalse(TEXT("Null world is never paused"), UFluxPrimePauseWorldSubsystem::IsWorldPaused(nullptr));
	TestEqual(TEXT("Null world has zero time"), UFluxPrimePauseWorldSubsystem::GetSimulationTime(nullptr), 0.0f);
	A->DestroyWorld(false);
	B->DestroyWorld(false);
	return true;
}
#endif
