// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/Components/FluxPrimeCrowdsNetComponent.h"

#include "StructUtils/InstancedStruct.h"
#include "Systems/FluxPrimeCrowdsRenderSystems.h"
#include "Systems/FluxPrimeGroundHeightSystems.h"
#include "Systems/FluxPrimeMovementSystems.h"

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
}

/*void UFluxPrimeCrowdsNetComponent::Initialize(uint16 crowdsTotal, TArray<TObjectPtr<UInstancedStaticMeshComponent>> crowdsComponents)
{
	CrowdsTotal = crowdsTotal;
	CrowdsComponents = crowdsComponents;
	
	CrowdsNets.Init(FFluxPrimeCrowdsNet(), CrowdsTotal);
	CrowdsTargetNets.Init(FFluxPrimeCrowdsTargetNet(), crowdsTotal);
	CrowdsAnimationNets.Init(FFluxPrimeCrowdsAnimationNet(), CrowdsTotal);
	CrowdsAccelerationsNets.Init(FFluxPrimeCrowdsAccelerationNet(), crowdsTotal);
}*/

void UFluxPrimeCrowdsNetComponent::Initialize(FFluxPrimeCrowdsNetComponentContext context)
{
	CrowdsTotal = context.crowdsTotal;
	CrowdsComponents = context.crowdsComponents;
	
	CrowdsNets.Init(FFluxPrimeCrowdsNet(), CrowdsTotal);
	CrowdsTargetNets.Init(FFluxPrimeCrowdsTargetNet(), context.crowdsTotal);
	CrowdsAnimationNets.Init(FFluxPrimeCrowdsAnimationNet(), CrowdsTotal);
	CrowdsAccelerationsNets.Init(FFluxPrimeCrowdsAccelerationNet(), context.crowdsTotal);
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
	CrowdsNets[indexSelected].NetID = data->IdPayload;
	
	CrowdsTargetNets[indexSelected].NetID = data->IdPayload;
	CrowdsTargetNets[indexSelected].NetTargetLocation = data->TargetLocationPayload;
	
	CrowdsAccelerationsNets[indexSelected].NetID = data->IdPayload;
	CrowdsAccelerationsNets[indexSelected].NetAcceleration = data->AccelerationPayload; 
	
	CrowdsAnimationNets[indexSelected].NetID = data->IdPayload;
	CrowdsAnimationNets[indexSelected].NetType = data->TypePayload;
	CrowdsAnimationNets[indexSelected].NetStart = data->StartFramePayload;
	CrowdsAnimationNets[indexSelected].NetEnd = data->EndFramePayload;
}
