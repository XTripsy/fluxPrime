#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Cores/FluxPrimeEnum.h"
#include "Crowds/Identity/CrowdsIdentity.h"
#include "StructUtils/InstancedStruct.h"
#include "Systems/FluxPrimeNavigationSystems.h"
#include "FluxPrimeCrowdsSpawnerModule.generated.h"

USTRUCT()
struct FFluxPrimeCrowdsSpawnerComponentContext
{
	GENERATED_BODY()
	
	uint16* crowdsActive = nullptr;
	uint16* crowdsTotal = nullptr;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* crowdsComponents = nullptr;
	
	TMap<FName, int8>* crowdsTypes = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	FFluxPrimeCrowds* crowdsData = nullptr;
	FFluxPrimeNavigationSystems* navigationSystems = nullptr;
};

DECLARE_DELEGATE_OneParam(FOnSpawnCrowdsNet, const FInstancedStruct& payload);

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsSpawnerModule
{
	GENERATED_BODY()
	
private:
	uint16* CrowdsActive = nullptr;
	uint16* CrowdsTotal = nullptr;
	FFluxPrimeCrowds* CrowdsData = nullptr;
	FFluxPrimeNavigationSystems* NavigationSystems = nullptr;
	
	TMap<FName, int8>* CrowdsTypes = nullptr;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* CrowdsComponents = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
public:
	FOnSpawnCrowdsNet OnSpawnCrowdsNet;
	
private:
	uint32 GetIndexSelected(uint32 typeCrowds)
	{
		for (int i = *CrowdsActive; i < *CrowdsTotal; ++i)
		{
			UE_LOG(LogTemp, Log, TEXT("SPAWNER COMPONENT:: Type: %d | DATATYPE: %d"), typeCrowds, CrowdsData->CrowdsType[i]);
			if (CrowdsData->CrowdsType[i] == typeCrowds)
			{
				UE_LOG(LogTemp, Log, TEXT("SPAWNER COMPONENT:: Type: %d | Index: %d"), typeCrowds, i);
				return i;
			}
		}
	
		return INDEX_NONE;
	}
	
	uint32 GetID(uint32 typeCrowds, uint32 indexSelected)
	{
		for (int i = indexSelected; i < *CrowdsTotal; ++i)
		{
			if (CrowdsData->CrowdsType[i] == typeCrowds)
			{
				int32 tempID = CrowdsData->CrowdsID[indexSelected];
				uint32 id = CrowdsData->CrowdsID[i];
				CrowdsData->CrowdsID[i] = tempID;
				return id;
			}
		}
	
		return INDEX_NONE;
	}

	
	uint32 GetPath(FVector location, uint32 indexSelected)
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

	void SetAnimationData(uint32 id, uint32 typeCrowds, uint32 indexSelected)
	{
		uint8 indexAnimation = static_cast<uint8>(CrowdsData->CrowdsState[indexSelected]);
		
		(*CrowdsComponents)[typeCrowds]->SetCustomDataValue(
				id,
				0,
				CrowdsData->CrowdsStartTimeAnimation[indexSelected],
				false
				);
		
		(*CrowdsComponents)[typeCrowds]->SetCustomDataValue(
				id,
				1,
				CrowdsData->CrowdsAnimationMapping[indexSelected].AnimationData[indexAnimation].AnimationStart,//CrowdsData->CrowdsAnimationData[indexSelected].AnimationOffset[indexAnimation],
				false
				);
		
		(*CrowdsComponents)[typeCrowds]->SetCustomDataValue(
				id,
				2,
				CrowdsData->CrowdsAnimationMapping[indexSelected].AnimationData[indexAnimation].AnimationEnd,//CrowdsData->CrowdsAnimationData[indexSelected].AnimationOffset[indexAnimation+1],
				false
				);
	}

	void SendCrowdsNetData(UCrowdsIdentity* identity, uint32 indexSelected, uint32 typeCrowds, int16 total)
	{
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
		
		FInstancedStruct instancedStruct = FInstancedStruct::Make(payload);
		
		OnSpawnCrowdsNet.ExecuteIfBound(instancedStruct);
	}
	
public:
	void Initialize(FFluxPrimeCrowdsSpawnerComponentContext context)
	{
		check(context.crowdsTypes);
		check(context.crowdsData);
		
		CrowdsActive = context.crowdsActive;
		CrowdsTotal = context.crowdsTotal;
		CrowdsComponents = context.crowdsComponents;
		CrowdsTypes = context.crowdsTypes;
		CrowdsData = context.crowdsData;
		NavigationSystems = context.navigationSystems;
		World = context.world;
	}

	void SpawnCrowd(UCrowdsIdentity* identity, FVector location, FRotator rotation)
	{
		if (!identity || !CrowdsTypes->Contains(identity->Identity)) return;
		if (*CrowdsActive == *CrowdsTotal) return;

		uint32 indexSelected = *CrowdsActive;
		uint32 typeCrowds = (*CrowdsTypes)[identity->Identity];
		uint32 indexDataSelected = GetIndexSelected(typeCrowds);
		
		if (indexSelected == INDEX_NONE) return;
		
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
		//CrowdsData->CrowdsState[indexSelected] = EFluxPrimeCrowdState::StateWalk;
		CrowdsData->CrowdsAnimationMapping[indexSelected] = CrowdsData->CrowdsAnimationMapping[indexDataSelected];
		CrowdsData->CrowdsIndexNavigationPath[indexSelected] = 0;
		CrowdsData->CrowdsTotalNavigationPath[indexSelected] = total;
		CrowdsData->CrowdsPreviousAnimationFrame[indexSelected] = -1.0f;
		//CrowdsData->CrowdsStartTimeAnimation[indexSelected] = World->GetRealTimeSeconds();

		CrowdsData->CrowdsAnimationMapping[indexDataSelected] = tempAnimationData;
		
		//SetAnimationData(id, typeCrowds, indexSelected);
		SendCrowdsNetData(identity, indexSelected, typeCrowds, total);
		
		++*CrowdsActive;
	}
};
