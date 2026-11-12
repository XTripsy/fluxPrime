#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeRenderSystem.generated.h"

USTRUCT(BlueprintType)
struct FRenderSystemDataTagConfig
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataLocationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRotationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataIdConfig;

	bool IsValid() const
	{
		return EntityDataLocationConfig.EntityDataTag.IsValid() 
		   && EntityDataRotationConfig.EntityDataTag.IsValid() 
		   && EntityDataIdConfig.EntityDataTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeRenderSystemsContext
{
	GENERATED_BODY()

	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* componentsISMC = nullptr;
	TArray<uint16>* catalogTotal = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeRenderSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()

private:
	TArray<TArray<FTransform>> TransformsPerComponent;

	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* ComponentsISMC = nullptr;
	
public:
	UPROPERTY(EditAnywhere)
	FRenderSystemDataTagConfig DataTagConfig;
	
	TArray<int16> EntityDataLocationID, EntityDataRotationID, EntityDataIdID, EntityDataTypeID;

private:
	void InitializedTransformData(const TArray<uint16>* catalogTotal)
	{
		TransformsPerComponent.Init(TArray<FTransform>(), catalogTotal->Num());
		FTransform transform;
		transform.SetLocation(FVector::One() * -100000.0f);
	
		for (int i = 0; i < TransformsPerComponent.Num(); ++i)
		{
			TransformsPerComponent[i].Init(transform, (*catalogTotal)[i]);
		}
	}

public:
	void Initialized(FFluxPrimeRenderSystemsContext context)
	{
		check(context.componentsISMC);
		check(context.catalogTotal);
		check(context.dataStores);
		
		int32 countData = context.dataStores->Num();
 		EntityDataLocationID.Init(0, countData);
 		EntityDataRotationID.Init(0, countData);
	
		ComponentsISMC = context.componentsISMC;
		InitializedTransformData(context.catalogTotal);
		
		int32 indexDataId = 0;
		for (auto& dataStore : *context.dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataLocationConfig, EntityDataLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRotationConfig, EntityDataRotationID[indexDataId]);
			indexDataId++;
		}
	}

	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Render_Systems);
		
		if (!IsAllDataValid)
		{
			UE_LOG(LogFluxPrime, Verbose, TEXT("RENDER SYSTEM :: data invalid; system skipped"));
			return;
		}
		
		auto& componentsISMC = *ComponentsISMC;
		int32 totalComponents = componentsISMC.Num();
		
		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
			FRotator* entityDataRotation = reinterpret_cast<FRotator*>(dataStore.GetRawBufferData(EntityDataRotationID[indexDataID]));
			
			check(entityDataLocation);
			check(entityDataRotation);

			int32 activeCount = dataStore.GetActiveEntityCount();
			int32 totalCount = dataStore.GetTotalEntityCount();

			for (int32 i = 0; i < activeCount; ++i)
			{
				check(indexDataID >= 0);
				check(indexDataID <= totalComponents);

				FTransform transform;
				transform.SetLocation(entityDataLocation[i]);
				FRotator Rot(0.0f, entityDataRotation[i].Yaw, 0.0f);
				transform.SetRotation(Rot.Quaternion());
				TransformsPerComponent[indexDataID][i] = transform;
			}

			for (int32 i = activeCount; i < totalCount; ++i)
			{
				FTransform transform;
				transform.SetLocation(FVector::One() * -100000.0f);
				TransformsPerComponent[indexDataID][i] = transform;
			}
			
			indexDataID++;
		}
		
		for (int32 i = 0; i < totalComponents; ++i)
		{
			componentsISMC[i]->BatchUpdateInstancesTransforms(0, TransformsPerComponent[i], false, false, false);
		}
	}
};