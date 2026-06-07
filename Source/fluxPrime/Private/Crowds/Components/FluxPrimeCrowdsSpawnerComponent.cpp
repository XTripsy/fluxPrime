// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/Components/FluxPrimeCrowdsSpawnerComponent.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Cores/FluxPrimeEnum.h"
#include "Crowds/Identity/CrowdsIdentity.h"
#include "StructUtils/InstancedStruct.h"

uint32 UFluxPrimeCrowdsSpawnerComponent::GetIndexSelected(uint32 typeCrowds)
{
	uint32 index = -1;

	for (int i = *CrowdsActive; i < *CrowdsTotal; ++i)
	{
		if (CrowdsData->CrowdsType[i] == typeCrowds)
		{
			index = i;
			UE_LOG(LogTemp, Log, TEXT("SPAWNER COMPONENT:: Type: %d | Index: %d"), typeCrowds, i);
			break;
		}
	}
	
	return index;
}

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
	//uint32 indexAnimation = CrowdsData->CrowdsAnimationIndex[indexSelected];
	uint8 indexAnimation = static_cast<uint8>(CrowdsData->CrowdsState[indexSelected]);
	
	/*CrowdsComponents[typeCrowds]->SetCustomDataValue(
			id,
			0,
			CrowdsData->CrowdsStartTimeAnimationFrame[indexSelected],
			false
			);
	
	CrowdsComponents[typeCrowds]->SetCustomDataValue(
			id,
			1,
			CrowdsData->CrowdsAnimationMapping[indexSelected].AnimationData[indexAnimation].AnimationStart,//CrowdsData->CrowdsAnimationData[indexSelected].AnimationOffset[indexAnimation],
			false
			);
	
	CrowdsComponents[typeCrowds]->SetCustomDataValue(
			id,
			2,
			CrowdsData->CrowdsAnimationMapping[indexSelected].AnimationData[indexAnimation].AnimationEnd,//CrowdsData->CrowdsAnimationData[indexSelected].AnimationOffset[indexAnimation+1],
			false
			);*/
}

void UFluxPrimeCrowdsSpawnerComponent::SendCrowdsNetData(UCrowdsIdentity* identity, uint32 indexSelected, uint32 typeCrowds, int16 total)
{
	//uint32 indexAnimation = CrowdsData->CrowdsAnimationIndex[indexSelected];
	uint8 indexAnimation = static_cast<uint8>(CrowdsData->CrowdsState[indexSelected]);
	
	FFluxPrimeOnSpawnCrowdsNetPayload payload;
	payload.LocationPayload = CrowdsData->CrowdsLocation[indexSelected];
	payload.RotationPayload = FRotator::CompressAxisToByte(CrowdsData->CrowdsRotation[indexSelected]);
	payload.TargetLocationPayload = (total > 0)? CrowdsData->CrowdsNavigationPath[indexSelected].LocationPaths[0] : FVector::ZeroVector;
	payload.AccelerationPayload = CrowdsData->CrowdsAcceleration[indexSelected];
	payload.IdPayload = CrowdsData->CrowdsID[indexSelected];
	payload.IndexSelectedPayload = indexSelected;
	payload.MaxSpeedPayload = identity->Speed;
	payload.TypePayload = typeCrowds;
	payload.StartFramePayload = CrowdsData->CrowdsAnimationMapping[indexSelected].AnimationData[indexAnimation].AnimationStart;
	payload.EndFramePayload = CrowdsData->CrowdsAnimationMapping[indexSelected].AnimationData[indexAnimation].AnimationEnd;
	
	//payload.StartFramePayload = CrowdsData->CrowdsAnimationData[indexSelected].AnimationOffset[indexAnimation];
	//payload.EndFramePayload = CrowdsData->CrowdsAnimationData[indexSelected].AnimationOffset[indexAnimation+1];
	
	FInstancedStruct instancedStruct = FInstancedStruct::Make(payload);
	
	OnSpawnCrowdsNet.ExecuteIfBound(instancedStruct);
}

/*void UFluxPrimeCrowdsSpawnerComponent::Initialize(uint16* crowdsActive, uint16* crowdsTotal, TArray<TObjectPtr<UInstancedStaticMeshComponent>> crowdsComponents,TMap<FName, int8> crowdsTypes, FFluxPrimeCrowds* crowdsData, FFluxPrimeNavigationSystems* navigationSystems)
{
	CrowdsActive = crowdsActive;
	CrowdsTotal = crowdsTotal;
	CrowdsComponents = crowdsComponents;
	CrowdsTypes = crowdsTypes;
	CrowdsData = crowdsData;
	NavigationSystems = navigationSystems;
}*/

/*void UFluxPrimeCrowdsSpawnerComponent::Initialize(FFluxPrimeCrowdsSpawnerComponentContext context)
{
	CrowdsActive = context.crowdsActive;
	CrowdsTotal = context.crowdsTotal;
	CrowdsComponents = context.crowdsComponents;
	CrowdsTypes = context.crowdsTypes;
	CrowdsData = context.crowdsData;
	NavigationSystems = context.navigationSystems;
}*/

void UFluxPrimeCrowdsSpawnerComponent::SpawnCrowd_Implementation(UCrowdsIdentity* identity, FVector location,
                                                                 FRotator rotation)
{
	if (!identity || !CrowdsTypes.Contains(identity->Identity)) return;
	if (*CrowdsActive == *CrowdsTotal) return;

	uint32 indexSelected = *CrowdsActive;
	uint32 typeCrowds = CrowdsTypes[identity->Identity];
	
	uint32 indexDataSelected = GetIndexSelected(typeCrowds);
	int32 id = GetID(typeCrowds, indexSelected);
	int16 total = GetPath(location, indexSelected);
	
	FFluxPrimeCrowdsAnimation tempAnimationData = CrowdsData->CrowdsAnimationMapping[indexSelected]; 
	
	CrowdsData->CrowdsLocation[indexSelected] = FVector(location.X, location.Y, 0);
	CrowdsData->CrowdsRotation[indexSelected] = rotation.Yaw;
	CrowdsData->CrowdsID[indexSelected] = id;
	CrowdsData->CrowdsType[indexSelected] = typeCrowds;
	CrowdsData->CrowdsMaxSpeed[indexSelected] = identity->Speed;
	CrowdsData->CrowdsDamage[indexSelected] = identity->Damage;
	CrowdsData->CrowdsHealth[indexSelected] = identity->Health;
	CrowdsData->CrowdsSize[indexSelected] = identity->Size;
	CrowdsData->CrowdsState[indexSelected] = EFluxPrimeCrowdState::StateWalk;
	CrowdsData->CrowdsAnimationMapping[indexSelected] = CrowdsData->CrowdsAnimationMapping[indexDataSelected];
	CrowdsData->CrowdsIndexNavigationPath[indexSelected] = 0;
	CrowdsData->CrowdsTotalNavigationPath[indexSelected] = total;
	//CrowdsData->CrowdsStartTimeAnimationFrame[indexSelected] = GetWorld()->GetRealTimeSeconds();

	CrowdsData->CrowdsAnimationMapping[indexDataSelected] = tempAnimationData;
	
	SetAnimationData(id, typeCrowds, indexSelected);
	SendCrowdsNetData(identity, indexSelected, typeCrowds, total);
	
	++*CrowdsActive;
}
