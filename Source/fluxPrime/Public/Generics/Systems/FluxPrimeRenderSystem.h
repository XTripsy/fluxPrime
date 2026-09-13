#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "FluxPrimeRenderSystem.generated.h"

USTRUCT(BlueprintType)
struct FRenderSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRotationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataIdTag;

	bool IsValid() const
	{
		return EntityDataLocationTag.IsValid() 
		   && EntityDataRotationTag.IsValid() 
		   && EntityDataIdTag.IsValid();
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
	
		for (int i = 0; i < TransformsPerComponent.Num(); ++i)
		{
			FTransform transform;
			transform.SetLocation(FVector::DownVector * 10000);
			TransformsPerComponent[i].Init(transform, (*catalogTotal)[i]);
		}
	}

public:
	void Initialized(FFluxPrimeRenderSystemsContext context)
	{
		check(context.componentsISMC);
		check(context.catalogTotal);
		check(context.dataStores);
	
		ComponentsISMC = context.componentsISMC;
		InitializedTransformData(context.catalogTotal);
		
		for (auto& dataStore : *context.dataStores)
		{
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataRotationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRotationTag));
		}
	}

	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		auto& componentsISMC = *ComponentsISMC;
		int32 totalComponents = componentsISMC.Num();
		
		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
			FRotator* entityDataRotation = reinterpret_cast<FRotator*>(dataStore.GetRawBufferData(EntityDataRotationID[indexDataID]));
			
			for (int32 i = 0; i < dataStore.GetActiveEntityCount(); ++i)
			{
				check(indexDataID >= 0);
				check(indexDataID <= totalComponents);

				FTransform transform;
				transform.SetLocation(entityDataLocation[i]);

				float unpackedYaw = entityDataRotation[i].Yaw;
				FRotator Rot(0.0f, unpackedYaw, 0.0f);
				transform.SetRotation(Rot.Quaternion());
			
				TransformsPerComponent[indexDataID][i] = transform;
			}
			
			indexDataID++;
		}
		
		for (int32 i = 0; i < totalComponents; ++i)
		{
			componentsISMC[i]->BatchUpdateInstancesTransforms(0, TransformsPerComponent[i], true, true, true);
		}
	}
};