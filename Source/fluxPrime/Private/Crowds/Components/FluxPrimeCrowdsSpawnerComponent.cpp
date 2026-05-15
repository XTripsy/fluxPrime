// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/Components/FluxPrimeCrowdsSpawnerComponent.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Crowds/Identity/CrowdsIdentity.h"
#include "StructUtils/InstancedStruct.h"

uint32 UFluxPrimeCrowdsSpawnerComponent::GetID(uint32 typeCrowds, uint32 indexSelected)
{
	uint32 id = -1;
	
	for (int i = indexSelected; i < *CrowdsTotal; ++i)
	{
		if (CrowdsData->CrowdsType[i] == typeCrowds)
		{
			int32 tempID = CrowdsData->CrowdsID[indexSelected];
			id = CrowdsData->CrowdsID[i];
			CrowdsData->CrowdsID[i] = tempID;
			break;
		}
	}
	
	return id;
}

uint32 UFluxPrimeCrowdsSpawnerComponent::GetPath(FVector location, uint32 indexSelected)
{
	if (!NavigationSystems) return 0;
	
	uint32 total = 0;
	TArray<FVector> path;
	
	if (NavigationSystems->IsActive)
	{
		if (!NavigationSystems->CalculatePath(location, FVector::ZeroVector, path)) return total;
		
		total = FMath::Min(path.Num() - 1, FluxConfig::NavigationArrayCount);
	    
		for (uint8 i = 0; i < total; ++i)
		{
			path[i+1].Z = 0;
			CrowdsData->CrowdsNavigationPath[indexSelected].LocationPaths[i] = path[i+1];
		}
	}
	
	return total;
}

void UFluxPrimeCrowdsSpawnerComponent::SetAnimationData(uint32 id, uint32 typeCrowds, uint32 indexSelected)
{
	uint32 indexAnimation = CrowdsData->CrowdsAnimationIndex[indexSelected];
	
	CrowdsComponents[typeCrowds]->SetCustomDataValue(
			id,
			0,
			CrowdsData->CrowdsStartTimeAnimationFrame[indexSelected],
			false
			);
	
	CrowdsComponents[typeCrowds]->SetCustomDataValue(
			id,
			1,
			CrowdsData->CrowdsAnimationData[indexSelected].AnimationOffset[indexAnimation],
			false
			);
	
	CrowdsComponents[typeCrowds]->SetCustomDataValue(
			id,
			2,
			CrowdsData->CrowdsAnimationData[indexSelected].AnimationOffset[indexAnimation+1],
			false
			);
}

void UFluxPrimeCrowdsSpawnerComponent::SendCrowdsNetData(UCrowdsIdentity* identity, uint32 indexSelected, uint32 typeCrowds, int16 total)
{
	uint32 indexAnimation = CrowdsData->CrowdsAnimationIndex[indexSelected];
	
	FFluxPrimeOnSpawnCrowdsNetPayload payload;
	payload.LocationPayload = CrowdsData->CrowdsLocation[indexSelected];
	payload.RotationPayload = FRotator::CompressAxisToByte(CrowdsData->CrowdsRotation[indexSelected]);
	payload.TargetLocationPayload = (total > 0)? CrowdsData->CrowdsNavigationPath[indexSelected].LocationPaths[0] : FVector::ZeroVector;
	payload.AccelerationPayload = CrowdsData->CrowdsAcceleration[indexSelected];
	payload.IdPayload = CrowdsData->CrowdsID[indexSelected];
	payload.IndexSelectedPayload = indexSelected;
	payload.MaxSpeedPayload = identity->Speed;
	payload.TypePayload = typeCrowds;
	payload.StartFramePayload = CrowdsData->CrowdsAnimationData[indexSelected].AnimationOffset[indexAnimation];
	payload.EndFramePayload = CrowdsData->CrowdsAnimationData[indexSelected].AnimationOffset[indexAnimation+1];
	
	FInstancedStruct instancedStruct = FInstancedStruct::Make(payload);
	
	OnSpawnCrowdsNet.ExecuteIfBound(instancedStruct);
}

void UFluxPrimeCrowdsSpawnerComponent::Initialize(uint16* crowdsActive, uint16* crowdsTotal, TArray<TObjectPtr<UInstancedStaticMeshComponent>> crowdsComponents,TMap<FName, int8> crowdsTypes, FFluxPrimeCrowds* crowdsData, FFluxPrimeNavigationSystems* navigationSystems)
{
	CrowdsActive = crowdsActive;
	CrowdsTotal = crowdsTotal;
	CrowdsComponents = crowdsComponents;
	CrowdsTypes = crowdsTypes;
	CrowdsData = crowdsData;
	NavigationSystems = navigationSystems;
}

void UFluxPrimeCrowdsSpawnerComponent::SpawnCrowd_Implementation(UCrowdsIdentity* identity, FVector location,
                                                                 FRotator rotation)
{
	if (!identity || !CrowdsTypes.Contains(identity->Identity)) return;
	if (*CrowdsActive == *CrowdsTotal) return;

	uint32 indexSelected = *CrowdsActive;
	uint32 typeCrowds = CrowdsTypes[identity->Identity];
	
	int32 id = GetID(typeCrowds, indexSelected);
	int16 total = GetPath(location, indexSelected);
	
	CrowdsData->CrowdsLocation[indexSelected] = FVector(location.X, location.Y, 0);
	CrowdsData->CrowdsRotation[indexSelected] = rotation.Yaw;
	CrowdsData->CrowdsID[indexSelected] = id;
	CrowdsData->CrowdsIndexNavigationPath[indexSelected] = 0;
	CrowdsData->CrowdsTotalNavigationPath[indexSelected] = total;
	CrowdsData->CrowdsStartTimeAnimationFrame[indexSelected] = GetWorld()->GetRealTimeSeconds();
	CrowdsData->CrowdsAnimationIndex[indexSelected] = 0;
	
	SetAnimationData(id, typeCrowds, indexSelected);
	SendCrowdsNetData(identity, indexSelected, typeCrowds, total);
	
	++*CrowdsActive;
}
