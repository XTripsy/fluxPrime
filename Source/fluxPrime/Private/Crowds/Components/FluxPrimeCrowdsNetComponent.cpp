// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/Components/FluxPrimeCrowdsNetComponent.h"

#include "StructUtils/InstancedStruct.h"
#include "Systems/FluxPrimeCrowdsRenderSystems.h"
#include "Systems/FluxPrimeGroundHeightSystems.h"
#include "Systems/FluxPrimeMovementSystems.h"

/*void UFluxPrimeCrowdsNetComponent::OnRep_CrowdData()
{
	int32 totalComponents = CrowdsComponents.Num();
	TArray<TArray<FTransform>> transformsPerComponent;
	transformsPerComponent.SetNum(totalComponents);
	
	for (int i = 0; i < CrowdsActive; ++i)
	{
		int8 typeIndex = CrowdsNets[i].NetType;
		if (typeIndex < 0 || typeIndex >= totalComponents) continue;

		FTransform transform;
		transform.SetLocation(CrowdsNets[i].NetLocation);

		float unpackedYaw = FRotator::DecompressAxisFromByte(CrowdsNets[i].NetRotation);
		FRotator Rot(0.0f, unpackedYaw, 0.0f);
		transform.SetRotation(Rot.Quaternion());
		
		transformsPerComponent[typeIndex].Add(transform);
	}
	
	for (int32 i = 0; i < totalComponents; ++i)
	{
		int32 NeededInstances = transformsPerComponent[i].Num();
		if (NeededInstances <= 0) continue;

		CrowdsComponents[i]->BatchUpdateInstancesTransforms(0, transformsPerComponent[i], true, true, true);
	}
}*/

UFluxPrimeCrowdsNetComponent::UFluxPrimeCrowdsNetComponent()
{
	SetIsReplicated(true);
}

void UFluxPrimeCrowdsNetComponent::OnRep_CrowdDataAnimation()
{
	for (int i = 0; i < CrowdsActive; ++i)
	{
		int32 type = CrowdsAnimationNets[i].NetType;
		int32 id = CrowdsAnimationNets[i].NetID;
		
		CrowdsComponents[type]->SetCustomDataValue(
			id,
			0,
			GetWorld()->GetRealTimeSeconds(),
			false
			);
	
		CrowdsComponents[type]->SetCustomDataValue(
				id,
				1,
				CrowdsAnimationNets[i].NetStart,
				false
				);
	
		CrowdsComponents[type]->SetCustomDataValue(
				id,
				2,
				CrowdsAnimationNets[i].NetEnd,
				false
				);
	}
}

void UFluxPrimeCrowdsNetComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFluxPrimeCrowdsNetComponent, CrowdsNets);
	DOREPLIFETIME(UFluxPrimeCrowdsNetComponent, CrowdsAnimationNets);
	//DOREPLIFETIME(UFluxPrimeCrowdsNetComponent, CrowdsAccelerationsNets);
	//DOREPLIFETIME(UFluxPrimeCrowdsNetComponent, CrowdsTargetNets);
}

void UFluxPrimeCrowdsNetComponent::Initialize(uint16 crowdsTotal, TArray<TObjectPtr<UInstancedStaticMeshComponent>> crowdsComponents)
{
	CrowdsTotal = crowdsTotal;
	CrowdsComponents = crowdsComponents;
	
	CrowdsNets.Init(FFluxPrimeCrowdsNet(), CrowdsTotal);
	CrowdsTargetNets.Init(FFluxPrimeCrowdsTargetNet(), crowdsTotal);
	CrowdsAnimationNets.Init(FFluxPrimeCrowdsAnimationNet(), CrowdsTotal);
	CrowdsAccelerationsNets.Init(FFluxPrimeCrowdsAccelerationNet(), crowdsTotal);
}

void UFluxPrimeCrowdsNetComponent::UpdateNetData(float DeltaTime, FFluxPrimeGroundHeightSystems& GroundHeightSystems,
	FFluxPrimeMovementSystems& MovementSystems, FFluxPrimeCrowdsRenderSystems& RenderSystems)
{
	GroundHeightSystems.UpdateNetGroundHeightSystems(DeltaTime, CrowdsNets, CrowdsActive);
	MovementSystems.UpdateNetMovementSystems(GetWorld(), DeltaTime, CrowdsNets, CrowdsTargetNets, CrowdsAccelerationsNets, CrowdsActive);
	RenderSystems.UpdateNetRenderCrowdsSystems(CrowdsComponents, CrowdsNets, CrowdsActive);
}

void UFluxPrimeCrowdsNetComponent::UpdateCrowdsData(const TArray<FVector_NetQuantize100>& accelerations, const TArray<FVector_NetQuantize100>& target)
{
	if (accelerations.IsEmpty() || target.IsEmpty()) return;
	
	for (int i = 0; i < CrowdsActive; ++i)
	{
		if (!accelerations.IsValidIndex(i) || !target.IsValidIndex(i)) continue;
		
		CrowdsAccelerationsNets[i].NetAcceleration = accelerations[i];
		CrowdsTargetNets[i].NetTargetLocation = target[i];
	}
}

/*void UFluxPrimeCrowdsNetComponent::UpdateCrowdsData(const TArray<int16>& id, const TArray<FVector>& accelerations,
                                                    const TArray<FVector>& target)
{
	if (id.IsEmpty() || accelerations.IsEmpty() || target.IsEmpty()) return;
	
	for (int i = 0; i < CrowdsActive; ++i)
	{
		uint16 indexSelected = id.IndexOfByKey(i);
		
		if (indexSelected != INDEX_NONE && accelerations.IsValidIndex(indexSelected) && target.IsValidIndex(indexSelected))
		{
			CrowdsAccelerationsNets[i].NetAcceleration = accelerations[indexSelected];
			CrowdsTargetNets[i].NetTargetLocation = target[indexSelected];
		}
	}
}*/

void UFluxPrimeCrowdsNetComponent::OnCrowdsActiveChange(uint16 count)
{
	CrowdsActive = count;
}

void UFluxPrimeCrowdsNetComponent::OnSpawnCrowdsData(const FInstancedStruct& payload)
{
	const FFluxPrimeOnSpawnCrowdsNetPayload* data = payload.GetPtr<FFluxPrimeOnSpawnCrowdsNetPayload>();
	if (!data) return;
	
	uint16 indexSelected = data->IndexSelectedPayload;
	
	CrowdsNets[indexSelected].NetLocation = data->LocationPayload;
	CrowdsNets[indexSelected].NetRotation = FRotator::CompressAxisToByte(data->RotationPayload);
	CrowdsNets[indexSelected].NetType = data->TypePayload;
	CrowdsNets[indexSelected].NetMaxSpeed = data->MaxSpeedPayload;
	//CrowdsNets[indexSelected].NetTargetLocation = data->TargetLocationPayload;
	
	CrowdsTargetNets[indexSelected].NetID = data->IdPayload;
	CrowdsTargetNets[indexSelected].NetTargetLocation = data->TargetLocationPayload;
	
	CrowdsAccelerationsNets[indexSelected].NetID = data->IdPayload;
	CrowdsAccelerationsNets[indexSelected].NetAcceleration = data->AccelerationPayload; 
	
	CrowdsAnimationNets[indexSelected].NetID = data->IdPayload;
	CrowdsAnimationNets[indexSelected].NetType = data->TypePayload;
	CrowdsAnimationNets[indexSelected].NetStart = data->StartFramePayload;
	CrowdsAnimationNets[indexSelected].NetEnd = data->EndFramePayload;
}
