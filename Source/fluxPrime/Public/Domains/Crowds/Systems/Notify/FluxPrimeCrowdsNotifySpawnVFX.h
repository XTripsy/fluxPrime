#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "NiagaraDataChannel.h"
#include "Domains/Crowds/FluxPrimeCrowdsVFXSubsystem.h"
#include "NiagaraSystem.h"
#include "Generics/Systems/Animation/FluxPrimeNotifyBase.h"
#include "NiagaraDataChannelPublic.h"
#include "Framework/Core/FluxPrimePauseWorldSubsystem.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Generics/Data/FluxPrimeAnimationData.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeCrowdsNotifySpawnVFX.generated.h"

USTRUCT(BlueprintType)
struct FNotifySpawnVfxDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRequestNotifyConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLocationConfig;
	
	bool IsValid() const
	{
		return EntityDataRequestNotifyConfig.EntityDataTag.IsValid()
			&& EntityDataLocationConfig.EntityDataTag.IsValid();
	}
};

USTRUCT()
struct FFluxPrimeCrowdsNotifySpawnVFXContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world = nullptr;
	
	FGetNotifyID getNotifyID = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsNotifySpawnVFX : public FFluxPrimeNotifyBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FNotifySpawnVfxDataTagConfig DataTagConfig;
	
	/** Persistent NDC listener, not a per-notify burst system. Must read DataChannel. */
	UPROPERTY(EditAnywhere, Category = "VFX")
	TObjectPtr<UNiagaraSystem> VFXSystem = nullptr;

	/** Global channel with a Position-typed variable. Owned listener is shared across notifies. */
	UPROPERTY(EditAnywhere, Category = "VFX")
	TObjectPtr<UNiagaraDataChannelAsset> DataChannel = nullptr;

	UPROPERTY(EditAnywhere, Category = "VFX")
	FName PositionVariable = TEXT("Position");
	
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "VFX", meta = (Units = "cm"))
	float TraceHeightOffset = 150.0f;

	TArray<int16> EntityDataRequestNotifyID, EntityDataLocationID;
	
private:
	bool HasValidChannelSchema() const
	{
		return UFluxPrimeCrowdsVFXSubsystem::IsConfigurationValid(VFXSystem, DataChannel, PositionVariable);
	}

public:
	void Initialized(FFluxPrimeCrowdsNotifySpawnVFXContext context)
	{
		check(context.world);
		check(context.getNotifyID);
		check(context.dataStores);
		
		int32 countData = context.dataStores->Num();
		EntityDataLocationID.Init(0, countData);
		EntityDataRequestNotifyID.Init(0, countData);
		
		World = context.world;
		NotifyID = context.getNotifyID(NotifyTag);

		int32 indexDataId = 0;
		for (auto& dataStore : *context.dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataLocationConfig, EntityDataLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRequestNotifyConfig, EntityDataRequestNotifyID[indexDataId]);
			indexDataId++;
		}

		if (!HasValidChannelSchema())
		{
			UE_LOG(LogFluxPrime, Warning, TEXT("NOTIFY SPAWN VFX :: assign an NDC listener VFXSystem and a Global Data Channel with one Position-typed '%s' variable."), *PositionVariable.ToString());
		}
	}
	
	void ExecuteNotify(const uint16 indexDataID, const uint16 indexMember, FFluxPrimeArchetypeDataStore& dataStore)
	{
		if (UFluxPrimePauseWorldSubsystem::IsWorldPaused(World)) return;
		if (!IsAllDataValid)
		{
			UE_LOG(LogFluxPrime, Verbose, TEXT("NOTIFY SPAWN VFX :: data invalid; system skipped"));
			return;
		}
		
		FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
		FFluxPrimeWrapperInt32* entityDataRequestNotify = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataRequestNotifyID[indexDataID]));
		
		check(entityDataLocation);
		check(entityDataRequestNotify);
		
		if (entityDataRequestNotify[indexMember].ValueInt32 != NotifyID) return;
		
		// Consume even when publication fails: cosmetic events must not replay on later frames.
		entityDataRequestNotify[indexMember].ValueInt32 = INDEX_NONE;
		if (!HasValidChannelSchema()) return;

		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_NotifyVFX_WriteDataChannel);
		const FVector Position = entityDataLocation[indexMember] + FVector::UpVector * TraceHeightOffset;
		if (IsValid(World))
		{
			if (auto* VFX = World->GetSubsystem<UFluxPrimeCrowdsVFXSubsystem>())
				VFX->Publish(VFXSystem, DataChannel, PositionVariable, Position);
		}
	}
};
