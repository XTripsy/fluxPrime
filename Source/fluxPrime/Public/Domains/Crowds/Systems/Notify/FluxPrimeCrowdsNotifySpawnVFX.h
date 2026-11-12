#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Generics/Systems/Animation/FluxPrimeNotifyBase.h"
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

	TArray<int16> EntityDataRequestNotifyID, EntityDataLocationID;
	
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
		
		UNiagaraComponent* projectileFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			VFXSystem,
			entityDataLocation[indexMember] + FVector::UpVector * 150,
			FRotator::ZeroRotator
		);
		
		entityDataRequestNotify[indexMember].ValueInt32 = INDEX_NONE;
	}
};
