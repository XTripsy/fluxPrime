#pragma once

#include "CoreMinimal.h"
#include "Generics/Systems/Animation/FluxPrimeNotifyBase.h"
#include "FluxPrimeCrowdsNotifySpawnVFX.generated.h"

USTRUCT(BlueprintType)
struct FNotifySpawnVfxDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRequestNotifyTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLocationTag;
	
	bool IsValid() const
	{
		return EntityDataRequestNotifyTag.IsValid()
		    && EntityDataLocationTag.IsValid();
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
		
		World = context.world;
		NotifyID = context.getNotifyID(NotifyTag);

		for (auto& dataStore : *context.dataStores)
		{
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataRequestNotifyID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestNotifyTag));
		}
	}
	
	void ExecuteNotify(const uint16 indexDataID, const uint16 indexMember, FFluxPrimeArchetypeDataStore& dataStore)
	{
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
