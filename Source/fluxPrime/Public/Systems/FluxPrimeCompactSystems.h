#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystems.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeCompactSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCompactSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool isDebug = false;
	
	FFluxPrimeCrowds* members = nullptr;
	uint16* memberActive = nullptr;
	TMap<FFluxPrimeCrowdsLookup, int32>* crowdsLookup = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCompactSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()
private:
	FFluxPrimeCrowds* Members = nullptr;
	uint16* MemberActive = nullptr;
	TMap<FFluxPrimeCrowdsLookup, int32>* CrowdsLookup = nullptr;
	
private:
	template<typename T>
	void SwapData(TArray<T>& Array, int32 currentIndex, int32 lastIndex)
	{
		Swap(Array[currentIndex], Array[lastIndex]);
	}
	
	void UpdateLookUpData(int32 lastIndex)
	{
		auto& members = *Members;
		FFluxPrimeCrowdsLookup key;
		key.CrowdsID = members.CrowdsID[lastIndex];
		key.CrowdsType = members.CrowdsType[lastIndex];
		int32& indexLookup = CrowdsLookup->FindOrAdd(key);
		indexLookup = lastIndex;
		UE_LOG(LogTemp, Log, TEXT("COMPACT:: LookUp %d"), indexLookup);
	}
	
	void SwapCrowdsData(int32 currentIndex, int32 lastIndex)
	{
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
		SwapData(members.CrowdsSize, currentIndex, lastIndex);
		SwapData(members.CrowdsState, currentIndex, lastIndex);
		SwapData(members.CrowdsNavigationPath, currentIndex, lastIndex);
		SwapData(members.CrowdsIndexNavigationPath, currentIndex, lastIndex);
		SwapData(members.CrowdsTotalNavigationPath, currentIndex, lastIndex);
		SwapData(members.CrowdsRequestNavigationPath, currentIndex, lastIndex);
		SwapData(members.CrowdsTargetLocation, currentIndex, lastIndex);
		SwapData(members.CrowdsCurrentTargetLocationPath, currentIndex, lastIndex);
		//SwapData(members.CrowdsAnimationMapping, currentIndex, lastIndex);
		SwapData(members.CrowdsAnimationState, currentIndex, lastIndex);
		SwapData(members.CrowdsRequestAnimationNotify, currentIndex, lastIndex);
		SwapData(members.CrowdsStartTimeAnimation, currentIndex, lastIndex);
		SwapData(members.CrowdsPreviousAnimationFrame, currentIndex, lastIndex);
	}
	
public:
	void InitializeCompactSystems(FFluxPrimeCompactSystemsContext context)
	{
		check(context.members);
		check(context.memberActive);
		check(context.crowdsLookup);
		
		Members = context.members;
		MemberActive = context.memberActive;
		CrowdsLookup = context.crowdsLookup;
	}
	
	void UpdateCompactSystems()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Compact_Systems);
		
		auto& members = *Members;
		int32 lastIndex = *MemberActive - 1;

		for (int32 index = 0; index <= lastIndex;)
		{
			if (!members.CrowdsCondition[index] && members.CrowdsRequestBackToPool[index])
			{
				SwapCrowdsData(index, lastIndex);
				--lastIndex;
				if (lastIndex >= 0) UpdateLookUpData(lastIndex);
			}
			else ++index;
		}

		*MemberActive = lastIndex + 1;
		UE_LOG(LogTemp, Log, TEXT("COMPACT:: Active %d"), *MemberActive);
	}
};