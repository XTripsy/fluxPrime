#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystem.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeCompactSystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCompactSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool isDebug = false;
	
	FFluxPrimeCrowds* members = nullptr;
	uint16* memberActive = nullptr;
	TArray<TArray<int16>>* crowdsPool = nullptr;
	TArray<int16>* crowdsHeadPool = nullptr;
	TMap<FFluxPrimeCrowdsLookup, int32>* crowdsLookup = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCompactSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
private:
	FFluxPrimeCrowds* Members = nullptr;
	uint16* MemberActive = nullptr;
	TArray<TArray<int16>>* CrowdsPool = nullptr;
	TArray<int16>* CrowdsHeadPool = nullptr;
	TMap<FFluxPrimeCrowdsLookup, int32>* CrowdsLookup = nullptr;
	
private:
	template<typename T>
	void SwapData(TArray<T>& Array, int32 currentIndex, int32 lastIndex)
	{
		Swap(Array[currentIndex], Array[lastIndex]);
	}
	
	void UpdateLookUpData(int32 index)
	{
		if (index < 0) return;
		
		auto& members = *Members;
		FFluxPrimeCrowdsLookup key;
		key.CrowdsID = members.CrowdsID[index];
		key.CrowdsType = members.CrowdsType[index];
		int32* indexLookup = CrowdsLookup->Find(key);
		*indexLookup = index;
	}
	
	void SwapPoolCrowdsData(int32 currentIndex, int32 selectedIndex, int32 previousHead, int32 currentHead, int32 type)
	{
		auto& members = *Members;
		auto& pool = *CrowdsPool;
		auto& headPool = *CrowdsHeadPool;
		
		int32 typeA = members.CrowdsType[currentIndex];
		int32 typeB = members.CrowdsType[selectedIndex];
		
		if (headPool[typeA] >= 0) pool[typeA][headPool[typeA]] = currentIndex;
		if (headPool[typeB] >= 0) pool[typeB][headPool[typeB]+1] = selectedIndex;
		if (previousHead >= 0 && currentHead >= 0) SwapData(pool[type], previousHead, currentHead);
	}
	
	void SwapCrowdsData(int32 currentIndex, int32 lastIndex)
	{
		if (currentIndex < 0 || lastIndex < 0) return;
		if (currentIndex == lastIndex) return;
		
		auto& members = *Members;
		SwapData(members.CrowdsLocation, currentIndex, lastIndex);
		SwapData(members.CrowdsRotation, currentIndex, lastIndex);
		SwapData(members.CrowdsVelocity, currentIndex, lastIndex);
		SwapData(members.CrowdsAcceleration, currentIndex, lastIndex);
		SwapData(members.CrowdsCellID, currentIndex, lastIndex);
		SwapData(members.CrowdsCondition, currentIndex, lastIndex);
		SwapData(members.CrowdsRequestBackToPool, currentIndex, lastIndex);
		SwapData(members.CrowdsMaxSpeed, currentIndex, lastIndex);
		SwapData(members.CrowdsType, currentIndex, lastIndex);
		SwapData(members.CrowdsID, currentIndex, lastIndex);
		SwapData(members.CrowdsHealth, currentIndex, lastIndex);
		SwapData(members.CrowdsDamage, currentIndex, lastIndex);
		SwapData(members.CrowdsAbilityRange, currentIndex, lastIndex);
		SwapData(members.CrowdsAbilityRange, currentIndex, lastIndex);
		SwapData(members.CrowdsSize, currentIndex, lastIndex);
		SwapData(members.CrowdsState, currentIndex, lastIndex);
		SwapData(members.CrowdsAnimationState, currentIndex, lastIndex);
		SwapData(members.CrowdsRequestAnimationNotify, currentIndex, lastIndex);
		SwapData(members.CrowdsStartTimeAnimation, currentIndex, lastIndex);
		SwapData(members.CrowdsPreviousAnimationFrame, currentIndex, lastIndex);
		SwapData(members.CrowdsPreviousLocation, currentIndex, lastIndex);
		SwapData(members.CrowdsCorridors, currentIndex, lastIndex);
		SwapData(members.CrowdsTargetID, currentIndex, lastIndex);
		SwapData(members.CrowdsTarget, currentIndex, lastIndex);
		SwapData(members.CrowdsCurrentTarget, currentIndex, lastIndex);
		SwapData(members.CrowdsLastReplanTarget, currentIndex, lastIndex);
		SwapData(members.CrowdsLastMoveTarget, currentIndex, lastIndex);
		SwapData(members.CrowdsLastOptimizeTime, currentIndex, lastIndex);
		SwapData(members.CrowdsLastMoveTargetTime, currentIndex, lastIndex);
		SwapData(members.CrowdsCountCorridor, currentIndex, lastIndex);
		SwapData(members.CrowdsRequestNeedReplan, currentIndex, lastIndex);
		SwapData(members.CrowdsWaypoints, currentIndex, lastIndex);
		SwapData(members.CrowdsCountWaypoints, currentIndex, lastIndex);
	}
	
public:
	void InitializeCompactSystems(FFluxPrimeCompactSystemsContext context)
	{
		check(context.members);
		check(context.memberActive);
		check(context.crowdsPool);
		check(context.crowdsHeadPool);
		check(context.crowdsLookup);
		
		Members = context.members;
		MemberActive = context.memberActive;
		CrowdsPool = context.crowdsPool;
		CrowdsHeadPool = context.crowdsHeadPool;
		CrowdsLookup = context.crowdsLookup;
	}
	
	void UpdateCompactSystems()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Compact_Systems);
		
		auto& members = *Members;
		auto& headPool = *CrowdsHeadPool;
		int32 lastIndex = *MemberActive - 1;

		for (int32 index = 0; index <= lastIndex;)
		{
			if (!members.CrowdsCondition[index] && members.CrowdsRequestBackToPool[index])
			{
				const int32 deadType = members.CrowdsType[index];
				const int32 previousHead = headPool[deadType];
				--headPool[deadType];
				const int32 currentHead = headPool[deadType];
				
				SwapCrowdsData(index, lastIndex);
				SwapPoolCrowdsData(index, lastIndex, previousHead, currentHead, deadType);
				UpdateLookUpData(lastIndex);
				UpdateLookUpData(index);
				--lastIndex;
			}
			else ++index;
		}
		
		*MemberActive = lastIndex + 1;
	}
};