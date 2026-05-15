#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Cores/FluxPrimeStruct.h"
#include "Systems/FluxPrimeBaseSystems.h"
#include "FluxPrimeCrowdsRenderSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsRenderSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()
	
public:
	void UpdateRenderCrowdsSystems(TArray<TObjectPtr<UInstancedStaticMeshComponent>>& crowdsComponents, FFluxPrimeCrowds& members, const int32 memberActive)
	{
		int32 totalComponents = crowdsComponents.Num();
		TArray<TArray<FTransform>> transformsPerComponent;
		transformsPerComponent.SetNum(totalComponents);
	
		for (int32 i = 0; i < memberActive; ++i)
		{
			int8 typeIndex = members.CrowdsType[i];
			if (typeIndex < 0 || typeIndex >= totalComponents) continue;

			FTransform transform;
			transform.SetLocation(members.CrowdsLocation[i]);

			float unpackedYaw = members.CrowdsRotation[i];
			FRotator Rot(0.0f, unpackedYaw, 0.0f);
			transform.SetRotation(Rot.Quaternion());
		
			transformsPerComponent[typeIndex].Add(transform);
		}

		for (int32 i = 0; i < totalComponents; ++i)
		{
			if (transformsPerComponent[i].Num() <= 0) continue;
		
			crowdsComponents[i]->BatchUpdateInstancesTransforms(0, transformsPerComponent[i], true, true, true);
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