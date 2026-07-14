#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Cores/FluxPrimeStruct.h"
#include "Systems/FluxPrimeBaseSystems.h"
#include "FluxPrimeCrowdsRenderSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsRenderSystemsContext
{
	GENERATED_BODY()
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* crowdsComponents = nullptr;
	TArray<FFluxPrimeCrowdsCatalog>* crowdsCatalog = nullptr;
	TArray<FVector>* locationCrowds = nullptr;
	TArray<float>* rotationCrowds = nullptr;
	TArray<int8>* typeCrowds = nullptr;
	uint16* memberActive = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsRenderSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()
	
private:
	TArray<TArray<FTransform>> TransformsPerComponent;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* CrowdsComponents = nullptr;
	TArray<FVector>* LocationCrowds = nullptr;
	TArray<float>* RotationCrowds = nullptr;
	TArray<int8>* TypeCrowds = nullptr;
	uint16* MemberActive = nullptr;
	
private:
	void Initialize(TArray<FFluxPrimeCrowdsCatalog>* catalog)
	{
		TransformsPerComponent.SetNum(catalog->Num());
		TransformsPerComponent.Init(TArray<FTransform>(), catalog->Num());
		
		for (int i = 0; i < TransformsPerComponent.Num(); ++i)
		{
			TransformsPerComponent[i].Reserve((*catalog)[i].CrowdsTotal);
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
		check(context.crowdsCatalog);
		
		CrowdsComponents = context.crowdsComponents;
		LocationCrowds = context.locationCrowds;
		RotationCrowds = context.rotationCrowds;
		TypeCrowds = context.typeCrowds;
		MemberActive = context.memberActive;
		Initialize(context.crowdsCatalog);
	}
	
	void UpdateRenderCrowdsSystems()
	{
		auto& crowdsComponents = *CrowdsComponents;
		auto& typeCrowds = *TypeCrowds;
		auto& locationCrowds = *LocationCrowds;
		auto& rotationCrowds = *RotationCrowds;
		
		int32 totalComponents = crowdsComponents.Num();
	
		for (int32 i = 0; i < *MemberActive; ++i)
		{
			int8 typeIndex = typeCrowds[i];
			if (typeIndex < 0 || typeIndex > totalComponents) continue;

			FTransform transform;
			transform.SetLocation(locationCrowds[i]);

			float unpackedYaw = rotationCrowds[i];
			FRotator Rot(0.0f, unpackedYaw, 0.0f);
			transform.SetRotation(Rot.Quaternion());
			
			TransformsPerComponent[typeIndex].Add(transform);
		}

		for (int32 i = 0; i < totalComponents; ++i)
		{
			if (TransformsPerComponent[i].Num() <= 0) continue;

			crowdsComponents[i]->BatchUpdateInstancesTransforms(0, TransformsPerComponent[i], true, true, true);
			TransformsPerComponent[i].Reset();
		}
	}
	
	void UpdateNetRenderCrowdsSystems(TArray<TObjectPtr<UInstancedStaticMeshComponent>>& crowdsComponents, TArray<FFluxPrimeCrowdsNet>& members, const int32 memberActive)
	{
		int32 totalComponents = crowdsComponents.Num();
		TArray<TArray<FTransform>> transformsPerComponent;
		transformsPerComponent.SetNum(totalComponents);
	
		for (int i = 0; i < memberActive; ++i)
		{
			int8 typeIndex = members[i].NetType;
			if (typeIndex < 0 || typeIndex >= totalComponents) continue;

			FTransform transform;
			transform.SetLocation(members[i].NetLocation);

			float unpackedYaw = FRotator::DecompressAxisFromByte(members[i].NetRotation);
			FRotator Rot(0.0f, unpackedYaw, 0.0f);
			transform.SetRotation(Rot.Quaternion());
		
			transformsPerComponent[typeIndex].Add(transform);
		}
	
		for (int32 i = 0; i < totalComponents; ++i)
		{
			int32 NeededInstances = transformsPerComponent[i].Num();
			if (NeededInstances <= 0) continue;

			crowdsComponents[i]->BatchUpdateInstancesTransforms(0, transformsPerComponent[i], true, true, true);
		}
	}
};