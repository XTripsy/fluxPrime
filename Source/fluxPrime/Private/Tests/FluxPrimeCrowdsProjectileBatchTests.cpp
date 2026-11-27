#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Domains/Crowds/FluxPrimeCrowdsProjectileBatchSubsystem.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileDamageHandler.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileTypes.h"
#include "Domains/Crowds/Systems/Ability/FluxPrimeCrowdsProjectileAbility.h"
#include "Tests/FluxPrimeCrowdsProjectileTestTarget.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraDataChannel.h"
#include "NiagaraDataChannel_Global.h"
#include "NiagaraDataChannel_Islands.h"
#include "NiagaraDataChannelCommon.h"
#include "NiagaraDataChannelHandler.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectIterator.h"

namespace
{
UWorld* FindProjectileBatchEditorWorld()
{
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Editor && Context.World() != nullptr)
		{
			return Context.World();
		}
	}
	return nullptr;
}

int32 CountProjectileBatchComponents(UWorld* World, UNiagaraSystem* Asset)
{
	int32 Count = 0;
	for (TObjectIterator<UNiagaraComponent> It; It; ++It)
	{
		if (!IsValid(*It))
		{
			continue;
		}

		if (It->GetWorld() == World && It->GetAsset() == Asset)
		{
			++Count;
		}
	}
	return Count;
}

UNiagaraDataChannelAsset* MakeProjectileTestChannel(bool bWrongSchema = false, bool bIslands = false)
{
	UNiagaraDataChannelAsset* Asset = NewObject<UNiagaraDataChannelAsset>();
	UNiagaraDataChannel* Channel = bIslands
		? static_cast<UNiagaraDataChannel*>(NewObject<UNiagaraDataChannel_Islands>(Asset))
		: NewObject<UNiagaraDataChannel_Global>(Asset);
	FindFieldChecked<FObjectPropertyBase>(Asset->GetClass(), TEXT("DataChannel"))->SetObjectPropertyValue_InContainer(Asset, Channel);
	FArrayProperty* Property = FindFieldChecked<FArrayProperty>(UNiagaraDataChannel::StaticClass(), TEXT("ChannelVariables"));
	TArray<FNiagaraDataChannelVariable>& Variables = *Property->ContainerPtrToValuePtr<TArray<FNiagaraDataChannelVariable>>(Channel);
	const auto AddVariable = [&Variables](FName Name, const FNiagaraTypeDefinition& Type)
	{
		FNiagaraDataChannelVariable& Variable = Variables.AddDefaulted_GetRef();
		Variable.SetName(Name);
		Variable.SetType(FNiagaraDataChannelVariable::ToDataChannelType(Type));
	};
	AddVariable(TEXT("Position"), bWrongSchema ? FNiagaraTypeDefinition::GetVec3Def() : FNiagaraTypeDefinition::GetPositionDef());
	AddVariable(TEXT("Direction"), FNiagaraTypeDefinition::GetVec3Def());
	AddVariable(TEXT("Orientation"), FNiagaraTypeDefinition::GetQuatDef());
	AddVariable(TEXT("Damage"), FNiagaraTypeDefinition::GetFloatDef());
	AddVariable(TEXT("ShotId"), FNiagaraTypeDefinition::GetIntDef());
	Channel->GetLayoutInfo();
	return Asset;
}

UNiagaraSystem* LoadProjectileBatchTestSystem()
{
	return LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/N_DummyDead.N_DummyDead"));
}

FFluxPrimeProjectileParticleData MakeProjectileTestParticle(const FVector& Position, float Damage, int32 ShotId)
{
	FFluxPrimeProjectileParticleData Particle;
	Particle.Position = Position;
	Particle.Velocity = FVector::ZeroVector;
	Particle.Size = 1.0f;
	Particle.Damage = Damage;
	Particle.ShotId = ShotId;
	return Particle;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeProjectileSpawnRecordTest, "FluxPrime.Medium.ProjectileBatch.SpawnRecord",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeProjectileSpawnRecordTest::RunTest(const FString& Parameters)
{
	const FFluxPrimeProjectileSpawnRecord Record = UFluxPrimeCrowdsProjectileBatchSubsystem::MakeSpawnRecord(
		FVector(100.0, 200.0, 0.0), FRotator(0.0, 90.0, 0.0), 42.0f, 75.0f, 3);

	TestTrue(TEXT("Spawn position applies the trace height offset on top of the muzzle location"),
		Record.Position.Equals(FVector(100.0, 200.0, 75.0), 0.01));
	TestTrue(TEXT("Spawn direction follows the shot rotation"),
		Record.Direction.Equals(FVector(0.0, 1.0, 0.0), 0.001));
	TestEqual(TEXT("Spawn record carries the shot damage"), Record.Damage, 42.0f);
	TestEqual(TEXT("Spawn record carries the shot id"), Record.ShotId, 3);
	const FRotator RolledRotation(21, 53, 87);
	const auto Rolled = UFluxPrimeCrowdsProjectileBatchSubsystem::MakeSpawnRecord(FVector::ZeroVector, RolledRotation, 42, 75, 3);
	TestTrue(TEXT("Muzzle local up retains roll"), Rolled.Orientation.RotateVector(FVector::UpVector).Equals(RolledRotation.RotateVector(FVector::UpVector), 0.0001));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeProjectileSingleComponentTest, "FluxPrime.Medium.ProjectileBatch.SingleComponent",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeProjectileSingleComponentTest::RunTest(const FString& Parameters)
{
	UWorld* World = FindProjectileBatchEditorWorld();
	if (World == nullptr)
	{
		AddError(TEXT("No editor world available for the projectile batch test."));
		return false;
	}

	UNiagaraSystem* BatchSystem = LoadProjectileBatchTestSystem();
	if (BatchSystem == nullptr)
	{
		AddError(TEXT("Unable to load /Game/N_DummyDead for the projectile batch test."));
		return false;
	}

	UFluxPrimeCrowdsProjectileBatchSubsystem* Subsystem = NewObject<UFluxPrimeCrowdsProjectileBatchSubsystem>(World);
	if (Subsystem == nullptr)
	{
		AddError(TEXT("Projectile batch subsystem was not created for the editor world."));
		return false;
	}

	Subsystem->ShutdownBatch();
	const int32 ComponentsBefore = CountProjectileBatchComponents(World, BatchSystem);

	FFluxPrimeProjectileBatchConfig Config;
	Config.System = BatchSystem;
	const auto Before = FFluxPrimeCrowdsProjectileDiagnostics::Get();
	TestFalse(TEXT("Missing channel rejects the shot"), Subsystem->SubmitShot(Config, {}));
	TestNull(TEXT("Missing channel creates no component"), Subsystem->GetBatchComponent());
	TestEqual(TEXT("Missing channel reason"), Subsystem->GetLastRejection(), EFluxPrimeProjectileRejection::MissingChannel);
	Config.SpawnChannel = NewObject<UNiagaraDataChannelAsset>();
	TestFalse(TEXT("Empty channel asset rejects the shot"), Subsystem->SubmitShot(Config, {}));
	Config.SpawnChannel = MakeProjectileTestChannel(true);
	TestFalse(TEXT("Vector Position is not a position schema"), Subsystem->SubmitShot(Config, {}));
	Config.SpawnChannel = MakeProjectileTestChannel(false, true);
	TestFalse(TEXT("Island channels cannot serve the origin batch"), Subsystem->SubmitShot(Config, {}));
	TestNull(TEXT("Invalid channels create no component"), Subsystem->GetBatchComponent());
	Config.SpawnChannel = MakeProjectileTestChannel();

	for (int32 Index = 0; Index < 50; ++Index)
	{
		FFluxPrimeProjectileShot Shot;
		Shot.Location = FVector(1000.0 + Index * 10.0, 2000.0, 300.0);
		Shot.Rotation = FRotator(21.0, Index * 3.0, 87.0);
		Shot.Damage = 10.0f;
		Shot.HeightOffset = 75.0f;
		TestTrue(TEXT("Valid global channel accepts shot"), Subsystem->SubmitShot(Config, Shot));
	}

	TestEqual(TEXT("Fifty shots share exactly one persistent Niagara component"),
		CountProjectileBatchComponents(World, BatchSystem) - ComponentsBefore, 1);
	TestNotNull(TEXT("The batch subsystem exposes the persistent component"), Subsystem->GetBatchComponent());

	UNiagaraComponent* OriginalComponent = Subsystem->GetBatchComponent();

	FFluxPrimeProjectileBatchConfig Incompatible = Config;
	Incompatible.SpawnChannel = MakeProjectileTestChannel();
	TestFalse(TEXT("Changing channels rejects rather than killing in-flight shots"), Subsystem->SubmitShot(Incompatible, {}));
	Incompatible = Config;
	Incompatible.System = NewObject<UNiagaraSystem>();
	TestFalse(TEXT("Changing systems rejects rather than killing in-flight shots"), Subsystem->SubmitShot(Incompatible, {}));
	TestEqual(TEXT("Rejected configuration preserves component identity"), Subsystem->GetBatchComponent(), OriginalComponent);
	const auto After = FFluxPrimeCrowdsProjectileDiagnostics::Get();
	TestEqual(TEXT("Submitted includes all attempts"), After.Submitted - Before.Submitted, uint64(56));
	TestEqual(TEXT("Accepted only counts published shots"), After.Accepted - Before.Accepted, uint64(50));
	TestEqual(TEXT("Fallback counts all six rejections"), After.Fallback - Before.Fallback, uint64(6));
	TestEqual(TEXT("Immediate write attempts"), After.Writes - Before.Writes, uint64(50));
	TestEqual(TEXT("Published is not consumed"), After.Published - Before.Published, uint64(50));
	for (const auto Reason : {EFluxPrimeProjectileRejection::MissingChannel, EFluxPrimeProjectileRejection::InvalidChannel,
		EFluxPrimeProjectileRejection::InvalidSchema, EFluxPrimeProjectileRejection::NonGlobalChannel})
	{
		const uint8 Index = static_cast<uint8>(Reason);
		TestEqual(FFluxPrimeCrowdsProjectileDiagnostics::ReasonName(Reason), After.Rejections[Index] - Before.Rejections[Index], uint64(1));
	}
	TestEqual(TEXT("Config conflict reason"), Subsystem->GetLastRejection(), EFluxPrimeProjectileRejection::ConfigurationConflict);

	UNiagaraDataChannelHandler* ChannelHandler = UNiagaraDataChannelLibrary::GetNiagaraDataChannel(World, Config.SpawnChannel);
	if (TestNotNull(TEXT("Real global channel handler exists"), ChannelHandler))
	{
		FNiagaraDataChannelDataPtr Data = ChannelHandler->FindData(FNiagaraDataChannelSearchParameters(), ENiagaraResourceAccess::ReadOnly);
		if (TestTrue(TEXT("Global channel data exists"), Data.IsValid()))
		{
			Data->ConsumePublishRequests(ChannelHandler, TG_PostUpdateWork);
			FNiagaraDataBufferRef Buffer = Data->GetCPUData(false);
			if (TestTrue(TEXT("CPU publication buffer exists"), Buffer != nullptr))
			{
				TestEqual(TEXT("All fifty shots published to Niagara CPU"), Buffer->GetNumInstances(), uint32(50));
				const FNiagaraDataSetCompiledData& Layout = Config.SpawnChannel->Get()->GetLayoutInfo()->GetDataSetCompiledData();
				const auto* Position = Layout.FindVariableLayoutInfo({FNiagaraTypeDefinition::GetPositionDef(), TEXT("Position")});
				const auto* Direction = Layout.FindVariableLayoutInfo({FNiagaraTypeDefinition::GetVec3Def(), TEXT("Direction")});
				const auto* Damage = Layout.FindVariableLayoutInfo({FNiagaraTypeDefinition::GetFloatDef(), TEXT("Damage")});
				const auto* ShotId = Layout.FindVariableLayoutInfo({FNiagaraTypeDefinition::GetIntDef(), TEXT("ShotId")});
				const auto* Orientation = Layout.FindVariableLayoutInfo({FNiagaraTypeDefinition::GetQuatDef(), TEXT("Orientation")});
				if (TestTrue(TEXT("Compiled CPU schema matches"), Position && Direction && Damage && ShotId && Orientation))
				{
					for (uint32 Index = 0; Index < Buffer->GetNumInstances(); ++Index)
					{
						TestEqual(TEXT("Position X survives publication"), *Buffer->GetInstancePtrFloat(Position->GetFloatComponentStart(), Index), 1000.0f + Index * 10.0f);
						TestEqual(TEXT("Position Z includes height offset"), *Buffer->GetInstancePtrFloat(Position->GetFloatComponentStart() + 2, Index), 375.0f);
						const FRotator ExpectedRotation(21.0, Index * 3.0, 87.0);
						const FVector ExpectedDirection = ExpectedRotation.Vector();
						const uint32 Q = Orientation->GetFloatComponentStart();
						const FQuat ActualOrientation(*Buffer->GetInstancePtrFloat(Q, Index), *Buffer->GetInstancePtrFloat(Q + 1, Index),
							*Buffer->GetInstancePtrFloat(Q + 2, Index), *Buffer->GetInstancePtrFloat(Q + 3, Index));
						TestTrue(TEXT("Full quaternion survives NDC publication"), ActualOrientation.Equals(ExpectedRotation.Quaternion(), 0.0001));
						TestTrue(TEXT("Direction survives publication"), FMath::IsNearlyEqual(*Buffer->GetInstancePtrFloat(Direction->GetFloatComponentStart(), Index), float(ExpectedDirection.X), 0.0001f));
						TestEqual(TEXT("Damage survives publication"), *Buffer->GetInstancePtrFloat(Damage->GetFloatComponentStart(), Index), 10.0f);
						TestEqual(TEXT("Rejected shots do not consume IDs"), *Buffer->GetInstancePtrInt32(ShotId->GetInt32ComponentStart(), Index), int32(Index));
					}
				}
			}
		}
	}

	Subsystem->ShutdownBatch();
	TestEqual(TEXT("Shutdown releases the persistent component"),
		CountProjectileBatchComponents(World, BatchSystem) - ComponentsBefore, 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeProjectilePerParticleDamageTest, "FluxPrime.Medium.ProjectileBatch.PerParticleDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeProjectilePerParticleDamageTest::RunTest(const FString& Parameters)
{
	UWorld* World = FindProjectileBatchEditorWorld();
	if (World == nullptr)
	{
		AddError(TEXT("No editor world available for the per-particle damage test."));
		return false;
	}

	const FVector TargetLocation(120000.0, 120000.0, 0.0);
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.ObjectFlags |= RF_Transient;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFluxPrimeCrowdsProjectileTestTarget* Target = World->SpawnActor<AFluxPrimeCrowdsProjectileTestTarget>(
		TargetLocation, FRotator::ZeroRotator, SpawnParameters);
	if (Target == nullptr)
	{
		AddError(TEXT("Unable to spawn the projectile damage test target."));
		return false;
	}

	UFluxPrimeCrowdsProjectileDamageHandler* Handler = NewObject<UFluxPrimeCrowdsProjectileDamageHandler>();
	Handler->Initialize(World);

	TArray<FFluxPrimeProjectileParticleData> Particles;
	Particles.Add(MakeProjectileTestParticle(TargetLocation, 10.0f, 1));
	Particles.Add(MakeProjectileTestParticle(TargetLocation, 25.0f, 2));
	Particles.Add(MakeProjectileTestParticle(TargetLocation + FVector(100000.0, 0.0, 0.0), 100.0f, 3));

	const auto BeforeDamage = FFluxPrimeCrowdsProjectileDiagnostics::Get();
	IFluxPrimeCrowdsProjectileCallbackHandler::Execute_ReceiveProjectileParticles(
		Handler, Particles, nullptr, FVector::ZeroVector);
	TestEqual(TEXT("All exported samples observed"), FFluxPrimeCrowdsProjectileDiagnostics::Get().ExportedSamples - BeforeDamage.ExportedSamples, uint64(3));
	TestEqual(TEXT("Damage observations match actual target events"), FFluxPrimeCrowdsProjectileDiagnostics::Get().DamageApplications - BeforeDamage.DamageApplications, uint64(2));

	TestEqual(TEXT("Each particle applies its own damage value"), Target->GetTotalDamage(), 35.0f);
	TestEqual(TEXT("Only particles that sweep into the target apply damage"), Target->GetDamageEventCount(), 2);

	Target->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeProjectileLegacyPathTest, "FluxPrime.Medium.ProjectileBatch.LegacyPath",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeProjectileLegacyPathTest::RunTest(const FString& Parameters)
{
	UWorld* World = FindProjectileBatchEditorWorld();
	if (World == nullptr)
	{
		AddError(TEXT("No editor world available for the projectile legacy path test."));
		return false;
	}

	UNiagaraSystem* PerShotSystem = LoadProjectileBatchTestSystem();
	if (PerShotSystem == nullptr)
	{
		AddError(TEXT("Unable to load /Game/N_DummyDead for the projectile legacy path test."));
		return false;
	}

	FFluxPrimeCrowdsProjectileAbilitySystemsContext Context;
	Context.world = World;

	FFluxPrimeCrowdsProjectileAbility Ability;
	Ability.InitializedProjectileAbilitySystems(Context);
	FObjectPropertyBase* LegacySystemProperty = FindFProperty<FObjectPropertyBase>(
		FFluxPrimeCrowdsProjectileAbility::StaticStruct(), TEXT("NiagaraSystem"));
	if (TestNotNull(TEXT("Serialized legacy system property remains available for fallback"), LegacySystemProperty))
	{
		LegacySystemProperty->SetObjectPropertyValue_InContainer(&Ability, PerShotSystem);
	}

	TSet<UNiagaraComponent*> ExistingComponents;
	for (TObjectIterator<UNiagaraComponent> It; It; ++It)
	{
		ExistingComponents.Add(*It);
	}
	const int32 ComponentsBefore = CountProjectileBatchComponents(World, PerShotSystem);

	Ability.Execute(FVector(3000.0, 4000.0, 500.0), FRotator::ZeroRotator, 5.0f);
	TestEqual(TEXT("Batching off keeps one spawned effect per shot"),
		CountProjectileBatchComponents(World, PerShotSystem) - ComponentsBefore, 1);

	// Batching requested but not configured must fall back to the per-shot path rather than drop the shot.
	Ability.bUseBatchedProjectiles = true;
	Ability.BatchedProjectileSystem = nullptr;
	Ability.ProjectileSpawnChannel = nullptr;

	Ability.Execute(FVector(3000.0, 4000.0, 500.0), FRotator::ZeroRotator, 5.0f);
	TestEqual(TEXT("An unconfigured batch falls back to one spawned effect per shot"),
		CountProjectileBatchComponents(World, PerShotSystem) - ComponentsBefore, 2);

	UFluxPrimeCrowdsProjectileBatchSubsystem* Batch = World->GetSubsystem<UFluxPrimeCrowdsProjectileBatchSubsystem>();
	Batch->ShutdownBatch();
	Ability.BatchedProjectileSystem = PerShotSystem;
	Ability.ProjectileSpawnChannel = MakeProjectileTestChannel();
	Ability.MuzzleFlashSystem = LoadObject<UNiagaraSystem>(nullptr,
		TEXT("/Game/ART/VFX/RangedCannon/Niagara/NS_Cannonball_Muzzle.NS_Cannonball_Muzzle"));
	if (!TestNotNull(TEXT("Production muzzle system exists"), Ability.MuzzleFlashSystem.Get())) return false;
	const int32 MuzzlesBefore = CountProjectileBatchComponents(World, Ability.MuzzleFlashSystem);
	const int32 BeforeAccepted = CountProjectileBatchComponents(World, PerShotSystem);
	Ability.Execute(FVector(3000.0, 4000.0, 500.0), FRotator(20, 30, 45), 5.0f);
	TestEqual(TEXT("Accepted shot creates one persistent projectile component"),
		CountProjectileBatchComponents(World, PerShotSystem) - BeforeAccepted, 1);
	UNiagaraComponent* PersistentProjectile = Batch->GetBatchComponent();
	PersistentProjectile->DeactivateImmediate();
	TestTrue(TEXT("Consumer completes before next shot"), PersistentProjectile->IsComplete());
	for (int32 Index = 0; Index < 10; ++Index)
	{
		Ability.Execute(FVector(3000, 4000, 500), FRotator(20, 30, 45), 5);
	}
	TestEqual(TEXT("Completed consumer is reused"), Batch->GetBatchComponent(), PersistentProjectile);
	TestFalse(TEXT("Next shot reactivates completed consumer"), PersistentProjectile->IsComplete());
	TestEqual(TEXT("Configured muzzle has exactly one persistent consumer across shots"),
		CountProjectileBatchComponents(World, Ability.MuzzleFlashSystem) - MuzzlesBefore, 1);
	Batch->ShutdownBatch();
	TestEqual(TEXT("Shutdown releases the muzzle consumer"),
		CountProjectileBatchComponents(World, Ability.MuzzleFlashSystem) - MuzzlesBefore, 0);

	for (TObjectIterator<UNiagaraComponent> It; It; ++It)
	{
		if (!ExistingComponents.Contains(*It) && It->GetWorld() == World && It->GetAsset() == PerShotSystem)
		{
			It->DestroyComponent();
		}
	}

	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeProjectileGraphDiagnosticsTest, "FluxPrime.Medium.ProjectileBatch.GraphDiagnostics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeProjectileGraphDiagnosticsTest::RunTest(const FString& Parameters)
{
	const auto Before = FFluxPrimeCrowdsProjectileDiagnostics::GetGraphCounts();
	FFluxPrimeCrowdsProjectileDiagnostics::RecordGraphCounts(3, 2, 1);
	const auto After = FFluxPrimeCrowdsProjectileDiagnostics::GetGraphCounts();
	TestEqual(TEXT("Consumed is independently observed"), After.Consumed - Before.Consumed, uint64(3));
	TestEqual(TEXT("Ball spawns are independently observed"), After.Balls - Before.Balls, uint64(2));
	TestEqual(TEXT("Muzzle bursts are independently observed"), After.Muzzles - Before.Muzzles, uint64(1));
	FFluxPrimeCrowdsProjectileDiagnostics::RecordGraphCounts(0, 0, 0);
	TestEqual(TEXT("Zero observations do not invent delivery"), FFluxPrimeCrowdsProjectileDiagnostics::GetGraphCounts().Consumed, After.Consumed);
	return true;
}
#endif
