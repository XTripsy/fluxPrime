#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "Crowds/Identity/CrowdsIdentity.h"
#include "StructUtils/InstancedStruct.h"
#include "FluxPrimeCrowdsSpawnerModule.generated.h"

USTRUCT()
struct FFluxPrimeCrowdsSpawnerComponentContext
{
	GENERATED_BODY()
	
	uint16* crowdsActive = nullptr;
	uint16* crowdsTotal = nullptr;
	
	TMap<FName, int8>* crowdsTypes = nullptr;
	TMap<FFluxPrimeCrowdsLookup, int32>* crowdsLookup = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	FFluxPrimeCrowds* crowdsData = nullptr;
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
	
	TMap<FName, int8>* CrowdsTypes = nullptr;
	TMap<FFluxPrimeCrowdsLookup, int32>* CrowdsLookup = nullptr;
	
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
		auto& members = *CrowdsData;
		
		for (int i = indexSelected; i < *CrowdsTotal; ++i)
		{
			if (members.CrowdsType[i] == typeCrowds)
			{
				int32 tempID = members.CrowdsID[indexSelected];
				uint32 id = members.CrowdsID[i];
				members.CrowdsID[i] = tempID;
				return id;
			}
		}
	
		return INDEX_NONE;
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
		check(context.crowdsLookup);
		
		CrowdsActive = context.crowdsActive;
		CrowdsTotal = context.crowdsTotal;
		CrowdsTypes = context.crowdsTypes;
		CrowdsData = context.crowdsData;
		CrowdsLookup = context.crowdsLookup;
		World = context.world;
	}

	void SpawnCrowd(UCrowdsIdentity* identity, FVector location, FRotator rotation)
	{
		if (!identity || !CrowdsTypes->Contains(identity->Identity)) return;
		if (*CrowdsActive == *CrowdsTotal) return;

		auto& members = *CrowdsData;
		auto& indentityData = *identity;
		uint32 indexSelected = *CrowdsActive;
		uint32 typeCrowds = (*CrowdsTypes)[identity->Identity];
		uint32 indexDataSelected = GetIndexSelected(typeCrowds);
		
		if (indexSelected == INDEX_NONE) return;
		
		int32 id = GetID(typeCrowds, indexSelected);
		
		FFluxPrimeCrowdsAnimation tempAnimationData = members.CrowdsAnimationMapping[indexSelected]; 
		
		members.CrowdsLocation[indexSelected] = FVector(location.X, location.Y, 0);
		members.CrowdsRotation[indexSelected] = rotation.Yaw;
		members.CrowdsID[indexSelected] = id;
		members.CrowdsType[indexSelected] = typeCrowds;
		members.CrowdsMaxSpeed[indexSelected] = indentityData.Speed;
		members.CrowdsDamage[indexSelected] = indentityData.Damage;
		members.CrowdsHealth[indexSelected] = indentityData.Health;
		members.CrowdsSize[indexSelected] = indentityData.Size;
		members.CrowdsCondition[indexSelected] = true;
		members.CrowdsRequestBackToPool[indexSelected] = false;
		members.CrowdsAnimationMapping[indexSelected] = members.CrowdsAnimationMapping[indexDataSelected];
		members.CrowdsIndexNavigationPath[indexSelected] = 0;
		members.CrowdsRequestNavigationPath[indexSelected] = true;
		members.CrowdsPreviousAnimationFrame[indexSelected] = -1.0f;

		members.CrowdsAnimationMapping[indexDataSelected] = tempAnimationData;
		
		FFluxPrimeCrowdsLookup key;
		key.CrowdsID = id;
		key.CrowdsType = typeCrowds;
		int32& indexLookup = CrowdsLookup->FindOrAdd(key);
		indexLookup = *CrowdsActive;
		//SendCrowdsNetData(identity, indexSelected, typeCrowds, total);
		
		++*CrowdsActive;
	}
};
