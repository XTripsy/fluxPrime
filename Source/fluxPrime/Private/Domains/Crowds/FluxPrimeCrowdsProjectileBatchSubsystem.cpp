#include "Domains/Crowds/FluxPrimeCrowdsProjectileBatchSubsystem.h"

#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileDamageHandler.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "Framework/Core/FluxPrimePauseWorldSubsystem.h"
#include "NiagaraDataChannel.h"
#include "NiagaraDataChannelAccessor.h"
#include "NiagaraDataChannel_Global.h"
#include "NiagaraSystem.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_INT_COUNTER(FluxProjectileBatchComponents, TEXT("FluxPrime/Projectile/LivePersistentComponents"));

namespace FluxPrimeProjectileBatchLocal
{
	static const FName PositionVarName(TEXT("Position"));
	static const FName DirectionVarName(TEXT("Direction"));
	static const FName OrientationVarName(TEXT("Orientation"));
	static const FName DamageVarName(TEXT("Damage"));
	static const FName ShotIdVarName(TEXT("ShotId"));
	static const FName CallbackParameterName(TEXT("User.ProjectileCallback"));

	bool HasSpawnSchema(const UNiagaraDataChannelAsset* Asset)
	{
		const UNiagaraDataChannel* Channel = IsValid(Asset) ? Asset->Get() : nullptr;
		// A component at the origin cannot consume records published into arbitrary islands.
		if (!IsValid(Channel) || !Channel->IsA<UNiagaraDataChannel_Global>())
		{
			return false;
		}

		const FNiagaraVariableBase Required[] = {
			{FNiagaraTypeDefinition::GetPositionDef(), PositionVarName},
			{FNiagaraDataChannelVariable::ToDataChannelType(FNiagaraTypeDefinition::GetVec3Def()), DirectionVarName},
			{FNiagaraDataChannelVariable::ToDataChannelType(FNiagaraTypeDefinition::GetFloatDef()), DamageVarName},
			{FNiagaraDataChannelVariable::ToDataChannelType(FNiagaraTypeDefinition::GetQuatDef()), OrientationVarName},
			{FNiagaraTypeDefinition::GetIntDef(), ShotIdVarName}
		};
		for (const FNiagaraVariableBase& Expected : Required)
		{
			int32 Matches = 0;
			for (const FNiagaraDataChannelVariable& Variable : Channel->GetVariables())
			{
				if (Variable.GetName() == Expected.GetName())
				{
					if (Variable.GetType() != Expected.GetType())
					{
						return false;
					}
					++Matches;
				}
			}
			if (Matches != 1)
			{
				return false;
			}
		}
		return true;
	}
}

FFluxPrimeProjectileSpawnRecord UFluxPrimeCrowdsProjectileBatchSubsystem::MakeSpawnRecord(const FVector& Location,
	const FRotator& Rotation, float Damage, float HeightOffset, int32 ShotId)
{
	FFluxPrimeProjectileSpawnRecord Record;
	Record.Position = Location + FVector::UpVector * HeightOffset;
	Record.Direction = Rotation.Vector();
	Record.Orientation = Rotation.Quaternion();
	Record.Damage = Damage;
	Record.ShotId = ShotId;
	return Record;
}

bool UFluxPrimeCrowdsProjectileBatchSubsystem::SubmitShot(const FFluxPrimeProjectileBatchConfig& Config,
	const FFluxPrimeProjectileShot& Shot)
{
	check(IsInGameThread());
	TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Projectile_Submit);
	const auto Reject = [this](EFluxPrimeProjectileRejection Reason)
	{
		LastRejection = Reason;
		FFluxPrimeCrowdsProjectileDiagnostics::RecordResult(Reason);
		return false;
	};
	if (!IsValid(GetWorld())) return Reject(EFluxPrimeProjectileRejection::InvalidWorld);
	if (auto* Pause = GetWorld()->GetSubsystem<UFluxPrimePauseWorldSubsystem>()) Pause->DrainPendingWork();
	if (GetWorld()->IsPaused()) return Reject(EFluxPrimeProjectileRejection::Paused);
	if (!IsValid(Config.System)) return Reject(EFluxPrimeProjectileRejection::MissingSystem);
	if (!IsValid(Config.SpawnChannel)) return Reject(EFluxPrimeProjectileRejection::MissingChannel);
	if (!IsValid(Config.SpawnChannel->Get())) return Reject(EFluxPrimeProjectileRejection::InvalidChannel);
	if (!Config.SpawnChannel->Get()->IsA<UNiagaraDataChannel_Global>()) return Reject(EFluxPrimeProjectileRejection::NonGlobalChannel);
	if (!FluxPrimeProjectileBatchLocal::HasSpawnSchema(Config.SpawnChannel)) return Reject(EFluxPrimeProjectileRejection::InvalidSchema);

	UNiagaraSystem* SeparateMuzzle = Config.MuzzleSystem != Config.System ? Config.MuzzleSystem.Get() : nullptr;
	if (IsValid(BatchComponent) && (BatchComponent->GetAsset() != Config.System || ActiveChannel != Config.SpawnChannel
		|| (IsValid(MuzzleComponent) ? MuzzleComponent->GetAsset() : nullptr) != SeparateMuzzle))
	{
		return Reject(EFluxPrimeProjectileRejection::ConfigurationConflict);
	}
	if (!EnsureBatchComponent(Config))
	{
		return Reject(EFluxPrimeProjectileRejection::ComponentUnavailable);
	}

	if (!DamageHandler->ReserveShot(NextShotId, Config.ReservationLifetimeSeconds, Config.MaxInFlightShots))
		return Reject(EFluxPrimeProjectileRejection::CapacityExceeded);

	const FFluxPrimeProjectileSpawnRecord Record = MakeSpawnRecord(
		Shot.Location, Shot.Rotation, Shot.Damage, Shot.HeightOffset, NextShotId);
	if (!PublishSpawnRecord(Config, Record))
	{
		DamageHandler->CancelShot(NextShotId);
		return Reject(EFluxPrimeProjectileRejection::WriterUnavailable);
	}
	LastRejection = EFluxPrimeProjectileRejection::None;
	FFluxPrimeCrowdsProjectileDiagnostics::RecordResult(LastRejection);
	DamageHandler->Initialize(GetWorld(), Shot.bDebug);
	NextShotId = NextShotId == MAX_int32 ? 0 : NextShotId + 1;
	return true;
}

void UFluxPrimeCrowdsProjectileBatchSubsystem::ShutdownBatch()
{
	check(IsInGameThread());
	if (auto* Pause = GetWorld() ? GetWorld()->GetSubsystem<UFluxPrimePauseWorldSubsystem>() : nullptr)
	{
		Pause->UnregisterNiagaraComponent(BatchComponent);
		Pause->UnregisterNiagaraComponent(MuzzleComponent);
	}
	if (IsValid(BatchComponent))
	{
		TRACE_COUNTER_DECREMENT(FluxProjectileBatchComponents);
		BatchComponent->DestroyComponent();
	}
	if (IsValid(MuzzleComponent))
	{
		TRACE_COUNTER_DECREMENT(FluxProjectileBatchComponents);
		MuzzleComponent->DestroyComponent();
	}
	MuzzleComponent = nullptr;
	BatchComponent = nullptr;
	if (DamageHandler) DamageHandler->Shutdown();
	DamageHandler = nullptr;
	ActiveChannel = nullptr;
}

void UFluxPrimeCrowdsProjectileBatchSubsystem::Deinitialize()
{
	ShutdownBatch();
	Super::Deinitialize();
}

bool UFluxPrimeCrowdsProjectileBatchSubsystem::EnsureBatchComponent(const FFluxPrimeProjectileBatchConfig& Config)
{
	if (IsValid(BatchComponent))
	{
		if (!BatchComponent->IsRegistered() || (IsValid(MuzzleComponent) && !MuzzleComponent->IsRegistered())) return false;
		// Completed instances cannot consume another record. Never reset a running instance's live particles.
		if (BatchComponent->IsComplete())
		{
			if (auto* Pause = GetWorld()->GetSubsystem<UFluxPrimePauseWorldSubsystem>()) Pause->RegisterNiagaraComponent(BatchComponent);
			BatchComponent->Activate(true);
		}
		if (IsValid(MuzzleComponent) && MuzzleComponent->IsComplete())
		{
			if (auto* Pause = GetWorld()->GetSubsystem<UFluxPrimePauseWorldSubsystem>()) Pause->RegisterNiagaraComponent(MuzzleComponent);
			MuzzleComponent->Activate(true);
		}
		return true;
	}

	UWorld* World = GetWorld();
	BatchComponent = NewObject<UNiagaraComponent>(World, NAME_None, RF_Transient);
	TRACE_COUNTER_INCREMENT(FluxProjectileBatchComponents);
	BatchComponent->SetAsset(Config.System);
	BatchComponent->SetAutoDestroy(false);
	BatchComponent->SetAutoActivate(false);
	BatchComponent->RegisterComponentWithWorld(World);
	if (!BatchComponent->IsRegistered())
	{
		ShutdownBatch();
		return false;
	}

	ActiveChannel = Config.SpawnChannel;
	DamageHandler = NewObject<UFluxPrimeCrowdsProjectileDamageHandler>(this);
	DamageHandler->Initialize(World, false);
	BatchComponent->SetVariableObject(FluxPrimeProjectileBatchLocal::CallbackParameterName, DamageHandler);
	if (auto* Pause = World->GetSubsystem<UFluxPrimePauseWorldSubsystem>()) Pause->RegisterNiagaraComponent(BatchComponent);
	BatchComponent->Activate(true);
	if (IsValid(Config.MuzzleSystem) && Config.MuzzleSystem != Config.System)
	{
		MuzzleComponent = NewObject<UNiagaraComponent>(World, NAME_None, RF_Transient);
		TRACE_COUNTER_INCREMENT(FluxProjectileBatchComponents);
		MuzzleComponent->SetAsset(Config.MuzzleSystem);
		MuzzleComponent->SetAutoDestroy(false);
		MuzzleComponent->SetAutoActivate(false);
		MuzzleComponent->RegisterComponentWithWorld(World);
		if (!MuzzleComponent->IsRegistered())
		{
			ShutdownBatch();
			return false;
		}
		if (auto* Pause = World->GetSubsystem<UFluxPrimePauseWorldSubsystem>()) Pause->RegisterNiagaraComponent(MuzzleComponent);
		MuzzleComponent->Activate(true);
	}
	return true;
}

bool UFluxPrimeCrowdsProjectileBatchSubsystem::PublishSpawnRecord(const FFluxPrimeProjectileBatchConfig& Config,
	const FFluxPrimeProjectileSpawnRecord& Record)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Projectile_NDCWrite);
	FFluxPrimeCrowdsProjectileDiagnostics::RecordWrite(false);
	FNiagaraDataChannelSearchParameters SearchParameters;
	SearchParameters.Location = Record.Position;
	UNiagaraDataChannelWriter* Writer = UNiagaraDataChannelLibrary::WriteToNiagaraDataChannel(
		GetWorld(), Config.SpawnChannel, SearchParameters, 1, false, true, false, TEXT("FluxPrimeProjectileBatch"));
	if (Writer == nullptr)
	{
		return false;
	}

	Writer->WritePosition(FluxPrimeProjectileBatchLocal::PositionVarName, 0, Record.Position);
	Writer->WriteVector(FluxPrimeProjectileBatchLocal::DirectionVarName, 0, Record.Direction);
	Writer->WriteFloat(FluxPrimeProjectileBatchLocal::DamageVarName, 0, Record.Damage);
	Writer->WriteInt(FluxPrimeProjectileBatchLocal::ShotIdVarName, 0, Record.ShotId);
	Writer->WriteQuat(FluxPrimeProjectileBatchLocal::OrientationVarName, 0, Record.Orientation);
	FFluxPrimeCrowdsProjectileDiagnostics::RecordWrite(true);
	return true;
}
