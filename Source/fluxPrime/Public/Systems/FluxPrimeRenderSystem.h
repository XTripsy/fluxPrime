#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Systems/FluxPrimeBaseSystem.h"
#include "FluxPrimeRenderSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsRenderSystemsContext
{
	GENERATED_BODY()
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* crowdsComponents = nullptr;
	TArray<uint16>* crowdsCatalogTotal = nullptr;
	uint16 crowdsCountCatalog;
	TArray<FVector>* locationCrowds = nullptr;
	TArray<float>* rotationCrowds = nullptr;
	TArray<int8>* typeCrowds = nullptr;
	TArray<int16>* instanceIndexCrowds = nullptr;
	uint16* memberActive = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeRenderSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	TArray<TArray<FTransform>> TransformsPerComponent;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* CrowdsComponents = nullptr;
	TArray<FVector>* LocationCrowds = nullptr;
	TArray<float>* RotationCrowds = nullptr;
	TArray<int8>* TypeCrowds = nullptr;
	TArray<int16>* InstanceIndexCrowds = nullptr;
	uint16* MemberActive = nullptr;
	
private:
	void Initialize(const uint16& crowdsCountCatalog, const TArray<uint16>* crowdsCatalogTotal)
	{
		TransformsPerComponent.Init(TArray<FTransform>(), crowdsCountCatalog);
		
		for (int i = 0; i < TransformsPerComponent.Num(); ++i)
		{
			FTransform transform;
			transform.SetLocation(FVector::DownVector * 10000);
			TransformsPerComponent[i].Init(transform, (*crowdsCatalogTotal)[i]);
		}
	}
	
public:
	void InitializeRenderSystems(FFluxPrimeCrowdsRenderSystemsContext context)
	{
		check(context.crowdsComponents);
		check(context.memberActive);
		check(context.locationCrowds);
		check(context.rotationCrowds);
		check(context.typeCrowds);
		check(context.crowdsCatalogTotal);
		check(context.instanceIndexCrowds);
		
		CrowdsComponents = context.crowdsComponents;
		LocationCrowds = context.locationCrowds;
		RotationCrowds = context.rotationCrowds;
		TypeCrowds = context.typeCrowds;
		MemberActive = context.memberActive;
		InstanceIndexCrowds = context.instanceIndexCrowds;
		Initialize(context.crowdsCountCatalog, context.crowdsCatalogTotal);
	}
	
	void UpdateRenderCrowdsSystems()
	{
		auto& crowdsComponents = *CrowdsComponents;
		auto& typeCrowds = *TypeCrowds;
		auto& locationCrowds = *LocationCrowds;
		auto& rotationCrowds = *RotationCrowds;
		auto& instanceIndexCrowds = *InstanceIndexCrowds;
		
		int32 totalComponents = crowdsComponents.Num();
	
		for (int32 i = 0; i < *MemberActive; ++i)
		{
			int8 typeIndex = typeCrowds[i];
			check(typeIndex >= 0);
			check(typeIndex <= totalComponents);

			FTransform transform;
			transform.SetLocation(locationCrowds[i]);

			float unpackedYaw = rotationCrowds[i];
			FRotator Rot(0.0f, unpackedYaw, 0.0f);
			transform.SetRotation(Rot.Quaternion());
			
			TransformsPerComponent[typeIndex][instanceIndexCrowds[i]] = transform;
		}

		for (int32 i = 0; i < totalComponents; ++i)
		{
			crowdsComponents[i]->BatchUpdateInstancesTransforms(0, TransformsPerComponent[i], true, true, true);
		}
	}
};