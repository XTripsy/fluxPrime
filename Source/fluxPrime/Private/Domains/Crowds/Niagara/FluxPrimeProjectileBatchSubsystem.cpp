#include "Domains/Crowds/Niagara/FluxPrimeProjectileBatchSubsystem.h"

#include "Domains/Crowds/Niagara/FluxPrimeProjectileDamageHandler.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraDataChannel.h"
#include "NiagaraDataChannelAccessor.h"
#include "NiagaraDataChannel_Global.h"
#include "NiagaraSystem.h"

namespace FluxPrimeProjectileBatchLocal
{
	static const FName PositionVarName(TEXT("Position"));
	static const FName DirectionVarName(TEXT("Direction"));
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

FFluxPrimeProjectileSpawnRecord UFluxPrimeProjectileBatchSubsystem::MakeSpawnRecord(const FVector& Location,
	const FRotator& Rotation, float Damage, float HeightOffset, int32 ShotId)
{
	FFluxPrimeProjectileSpawnRecord Record;
	Record.Position = Location + FVector::UpVector * HeightOffset;
	Record.Direction = Rotation.Vector();
	Record.Damage = Damage;
	Record.ShotId = ShotId;
	return Record;
}

bool UFluxPrimeProjectileBatchSubsystem::SubmitShot(const FFluxPrimeProjectileBatchConfig& Config,
	const FFluxPrimeProjectileShot& Shot)
{
	check(IsInGameThread());
	if (!IsValid(GetWorld()) || !IsValid(Config.System) || !FluxPrimeProjectileBatchLocal::HasSpawnSchema(Config.SpawnChannel))
	{
		return false;
	}

	if (IsValid(BatchComponent) && (BatchComponent->GetAsset() != Config.System || ActiveChannel != Config.SpawnChannel))
	{
		// Keep existing projectiles alive; the caller can spawn this shot through the legacy path.
		return false;
	}
	if (!EnsureBatchComponent(Config))
	{
		return false;
	}

	const FFluxPrimeProjectileSpawnRecord Record = MakeSpawnRecord(
		Shot.Location, Shot.Rotation, Shot.Damage, Shot.HeightOffset, NextShotId);
	if (!PublishSpawnRecord(Config, Record))
	{
		return false;
	}
	DamageHandler->Initialize(GetWorld(), Shot.bDebug);
	NextShotId = NextShotId == MAX_int32 ? 0 : NextShotId + 1;
	return true;
}

void UFluxPrimeProjectileBatchSubsystem::ShutdownBatch()
{
	check(IsInGameThread());
	if (IsValid(BatchComponent))
	{
		BatchComponent->DestroyComponent();
	}
	BatchComponent = nullptr;
	DamageHandler = nullptr;
	ActiveChannel = nullptr;
}

void UFluxPrimeProjectileBatchSubsystem::Deinitialize()
{
	ShutdownBatch();
	Super::Deinitialize();
}

bool UFluxPrimeProjectileBatchSubsystem::EnsureBatchComponent(const FFluxPrimeProjectileBatchConfig& Config)
{
	if (IsValid(BatchComponent))
	{
		return BatchComponent->IsRegistered();
	}

	UWorld* World = GetWorld();
	BatchComponent = NewObject<UNiagaraComponent>(World, NAME_None, RF_Transient);
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
	DamageHandler = NewObject<UFluxPrimeProjectileDamageHandler>(this);
	DamageHandler->Initialize(World, false);
	BatchComponent->SetVariableObject(FluxPrimeProjectileBatchLocal::CallbackParameterName, DamageHandler);
	BatchComponent->Activate(true);
	return true;
}

bool UFluxPrimeProjectileBatchSubsystem::PublishSpawnRecord(const FFluxPrimeProjectileBatchConfig& Config,
	const FFluxPrimeProjectileSpawnRecord& Record)
{
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
	return true;
}
