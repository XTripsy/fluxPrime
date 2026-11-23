#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "NiagaraFunctionLibrary.h"
#include "Generics/Systems/Animation/FluxPrimeNotifyBase.h"
#include "NiagaraSystem.h"
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
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> VFXSystem = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "VFX", meta = (Units = "cm"))
	float TraceHeightOffset = 150.0f;

	TArray<int16> EntityDataRequestNotifyID, EntityDataLocationID;
	
private:
	/**
	 * Editor builds defer Niagara's on-demand compile until the system is first activated, and
	 * UNiagaraComponent::Activate then flushes that request synchronously on the game thread.
	 * That flush is what stalled notify dispatch in the 1000-entity capture. Performing it here,
	 * during the manager's loading phase, keeps the cost out of gameplay frames.
	 * Cooked builds ship compiled data and never enter the on-demand path.
	 */
	void PrepareVFXSystemCompilation()
	{
#if WITH_EDITOR
		if (!VFXSystem) return;
		if (!VFXSystem->NeedsRequestCompile() && !VFXSystem->HasActiveCompilations()) return;

		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_NotifyVFX_PrepareCompile);
		const double startSeconds = FPlatformTime::Seconds();

		// VM compilation only; GPU shader compilation stays on its normal async path.
		VFXSystem->WaitForCompilationComplete(false, false);

		UE_LOG(LogFluxPrime, Log,
			TEXT("NOTIFY SPAWN VFX :: '%s' prepared in %.3f ms during loading (pendingCompile=%d, readyToRun=%d)"),
			*GetPathNameSafe(VFXSystem),
			(FPlatformTime::Seconds() - startSeconds) * 1000.0,
			VFXSystem->NeedsRequestCompile() ? 1 : 0,
			VFXSystem->IsReadyToRun() ? 1 : 0);

		if (VFXSystem->NeedsRequestCompile())
		{
			UE_LOG(LogFluxPrime, Warning,
				TEXT("NOTIFY SPAWN VFX :: '%s' still has a pending compile after the loading-phase flush; its first dispatch can still stall."),
				*GetPathNameSafe(VFXSystem));
		}
#endif
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

		// Only payloads that can actually dispatch need the effect ready.
		if (IsAllDataValid) PrepareVFXSystemCompilation();
	}
	
	void ExecuteNotify(const uint16 indexDataID, const uint16 indexMember, FFluxPrimeArchetypeDataStore& dataStore)
	{
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
		
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_NotifyVFX_Spawn);
		UNiagaraComponent* projectileFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			VFXSystem,
			entityDataLocation[indexMember] + FVector::UpVector * TraceHeightOffset,
			FRotator::ZeroRotator
		);
		
		entityDataRequestNotify[indexMember].ValueInt32 = INDEX_NONE;
	}
};
