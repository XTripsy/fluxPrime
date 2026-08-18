#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "Crowds/Identity/FluxPrimeCrowdsIdentity.h"
#include "FluxPrimeCrowdsSpawnerModule.generated.h"

USTRUCT()
struct FFluxPrimeCrowdsSpawnerComponentContext
{
	GENERATED_BODY()
	
	uint16* crowdsActive = nullptr;
	uint16* crowdsTotal = nullptr;
	
	TArray<TArray<int16>>* crowdsPool = nullptr;
	TArray<int16>* crowdsHeadPool = nullptr;
	TMap<FName, int8>* crowdsTypes = nullptr;
	TMap<FFluxPrimeCrowdsLookup, int32>* crowdsLookup = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	FFluxPrimeCrowds* crowdsData = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsSpawnerModule
{
	GENERATED_BODY()
	
private:
	uint16* CrowdsActive = nullptr;
	uint16* CrowdsTotal = nullptr;
	FFluxPrimeCrowds* CrowdsData = nullptr;
	
	TArray<TArray<int16>>* CrowdsPool = nullptr;
	TArray<int16>* CrowdsHeadPool = nullptr;
	TMap<FName, int8>* CrowdsTypes = nullptr;
	TMap<FFluxPrimeCrowdsLookup, int32>* CrowdsLookup = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
private:
	uint32 GetIndexSelected(uint32 typeCrowds)
	{
		auto& type = CrowdsData->CrowdsType; 
		
		for (int i = *CrowdsActive; i < *CrowdsTotal; ++i)
		{
			if (type[i] == typeCrowds)
			{
				return i;
			}
		}
	
		return INDEX_NONE;
	}
	
	template<typename T>
	void SwapData(TArray<T>& Array, int32 currentIndex, int32 selectedIndex)
	{
		Swap(Array[currentIndex], Array[selectedIndex]);
	}
	
	void UpdateLookUpData(int32 index)
	{
		auto& members = *CrowdsData;
		FFluxPrimeCrowdsLookup key;
		key.CrowdsID = members.CrowdsID[index];
		key.CrowdsType = members.CrowdsType[index];
		int32* indexLookup = CrowdsLookup->Find(key);
		*indexLookup = index;
	}
	
	void SwapPoolCrowdsData(int32 currentIndex, int32 selectedIndex, int32 previousHead, int32 currentHead, int32 type)
	{
		auto& members = *CrowdsData;
		auto& pool = *CrowdsPool;
		
		pool[members.CrowdsType[currentIndex]][members.CrowdsID[currentIndex]] = currentIndex;
		pool[members.CrowdsType[selectedIndex]][members.CrowdsID[selectedIndex]] = selectedIndex;
		
		SwapData(pool[type], previousHead, currentHead);
	}
	
	void UpdatePoolCrwodsData(int32 currentIndex, int32 selectedIndex)
	{
		if (currentIndex == selectedIndex) return;
		
		auto& members = *CrowdsData;
		auto& pool = *CrowdsPool;
		auto& headPool = *CrowdsHeadPool;
		
		int32 typeA = members.CrowdsType[currentIndex];
		int32 typeB = members.CrowdsType[selectedIndex];
		
		if (headPool[typeA] >= 0) pool[typeA][headPool[typeA]] = currentIndex;
		if (headPool[typeB] >= 0) pool[typeB][headPool[typeB]+1] = selectedIndex;
	}
	
	void SwapCrowdsData(int32 currentIndex, int32 selectedIndex)
	{
		if (currentIndex == selectedIndex) return;
		
		auto& members = *CrowdsData;
		SwapData(members.CrowdsLocation, currentIndex, selectedIndex);
		SwapData(members.CrowdsRotation, currentIndex, selectedIndex);
		SwapData(members.CrowdsVelocity, currentIndex, selectedIndex);
		SwapData(members.CrowdsAcceleration, currentIndex, selectedIndex);
		SwapData(members.CrowdsCellID, currentIndex, selectedIndex);
		SwapData(members.CrowdsCondition, currentIndex, selectedIndex);
		SwapData(members.CrowdsRequestBackToPool, currentIndex, selectedIndex);
		SwapData(members.CrowdsMaxSpeed, currentIndex, selectedIndex);
		SwapData(members.CrowdsType, currentIndex, selectedIndex);
		SwapData(members.CrowdsID, currentIndex, selectedIndex);
		SwapData(members.CrowdsHealth, currentIndex, selectedIndex);
		SwapData(members.CrowdsDamage, currentIndex, selectedIndex);
		SwapData(members.CrowdsAbilityRange, currentIndex, selectedIndex);
		SwapData(members.CrowdsRequestAbility, currentIndex, selectedIndex);
		SwapData(members.CrowdsSize, currentIndex, selectedIndex);
		SwapData(members.CrowdsState, currentIndex, selectedIndex);
		SwapData(members.CrowdsAnimationState, currentIndex, selectedIndex);
		SwapData(members.CrowdsRequestAnimationNotify, currentIndex, selectedIndex);
		SwapData(members.CrowdsStartTimeAnimation, currentIndex, selectedIndex);
		SwapData(members.CrowdsPreviousAnimationFrame, currentIndex, selectedIndex);
		SwapData(members.CrowdsPreviousLocation, currentIndex, selectedIndex);
		SwapData(members.CrowdsCorridors, currentIndex, selectedIndex);
		SwapData(members.CrowdsTargetID, currentIndex, selectedIndex);
		SwapData(members.CrowdsTarget, currentIndex, selectedIndex);
		SwapData(members.CrowdsCurrentTarget, currentIndex, selectedIndex);
		SwapData(members.CrowdsLastReplanTarget, currentIndex, selectedIndex);
		SwapData(members.CrowdsLastOptimizeTime, currentIndex, selectedIndex);
		SwapData(members.CrowdsLastMoveTargetTime, currentIndex, selectedIndex);
		SwapData(members.CrowdsCountCorridor, currentIndex, selectedIndex);
		SwapData(members.CrowdsRequestNeedReplan, currentIndex, selectedIndex);
		SwapData(members.CrowdsLastMoveTarget, currentIndex, selectedIndex);
		SwapData(members.CrowdsWaypoints, currentIndex, selectedIndex);
		SwapData(members.CrowdsCountWaypoints, currentIndex, selectedIndex);
	}

public:
	void Initialize(FFluxPrimeCrowdsSpawnerComponentContext context)
	{
		check(context.crowdsTypes);
		check(context.crowdsData);
		check(context.crowdsPool);
		check(context.crowdsHeadPool);
		check(context.crowdsLookup);
		
		CrowdsActive = context.crowdsActive;
		CrowdsTotal = context.crowdsTotal;
		CrowdsTypes = context.crowdsTypes;
		CrowdsPool = context.crowdsPool;
		CrowdsHeadPool = context.crowdsHeadPool;
		CrowdsData = context.crowdsData;
		CrowdsLookup = context.crowdsLookup;
		World = context.world;
	}
	
	void SpawnCrowd(UFluxPrimeCrowdsIdentity* identity, FVector location, FRotator rotation)
	{
		if (!identity || !CrowdsTypes->Contains(identity->Identity)) return;
		if (*CrowdsActive >= *CrowdsTotal) return;

		auto& members = *CrowdsData;
		auto& type = *CrowdsTypes;
		auto& pool = *CrowdsPool;
		auto& headPool = *CrowdsHeadPool;
		
		uint8 health = identity->Health;
		uint32 indexSelected = *CrowdsActive;
		uint32 typeCrowds = type[identity->Identity];
		++headPool[typeCrowds];
		const int32 currentHead = headPool[typeCrowds];
		
		if (indexSelected == INDEX_NONE || !pool[typeCrowds].IsValidIndex(currentHead)) return;
		
		uint32 indexToSwap = pool[typeCrowds][currentHead];
		
		SwapCrowdsData(indexSelected, indexToSwap);
		UpdatePoolCrwodsData(indexSelected, indexToSwap);
		UpdateLookUpData(indexSelected);
		UpdateLookUpData(indexToSwap);
		
		members.CrowdsRequestAnimationNotify[indexSelected] = EFluxPrimeCrowdAnimationNotify::NotifyNone;
		members.CrowdsCondition[indexSelected] = true;
		members.CrowdsRequestBackToPool[indexSelected] = false;
		members.CrowdsState[indexSelected] = EFluxPrimeCrowdState::StateWalk;
		members.CrowdsLocation[indexSelected] = FVector(location.X, location.Y, 0);
		members.CrowdsPreviousLocation[indexSelected] = members.CrowdsLocation[indexSelected];
		members.CrowdsTarget[indexSelected] = members.CrowdsLocation[indexSelected];
		members.CrowdsCurrentTarget[indexSelected] = members.CrowdsLocation[indexSelected];
		members.CrowdsRotation[indexSelected] = rotation.Yaw;
		members.CrowdsHealth[indexSelected] = health;
		members.CrowdsCountCorridor[indexSelected] = 0;
		members.CrowdsCorridors[indexSelected] = FFluxPrimeCrowdsCorridor();
		members.CrowdsRequestNeedReplan[indexSelected] = true;
		members.CrowdsPreviousAnimationFrame[indexSelected] = -1.0f;
		
		++*CrowdsActive;
	}
};
