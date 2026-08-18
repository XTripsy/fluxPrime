#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Cores/FluxPrimeAnimationData.h"
#include "Cores/FluxPrimeStruct.h"
#include "Crowds/Identity/FluxPrimeCrowdsIdentity.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Systems/FluxPrimeStateMachineSystem.h"
#include "FluxPrimeCrowdsDataModule.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsDataModuleContext
{
	GENERATED_BODY()
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* crowdsComponents = nullptr;
	TArray<FFluxPrimeCrowdsCatalog>* crowdsCatalog;
	
	TArray<TArray<int16>>* crowdsPool = nullptr;
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>>* crowdsAnimationSoftRef = nullptr;
	TMap<FFluxPrimeCrowdsLookup, int32>* crowdsLookup = nullptr;
	
	uint16* crowdsTotal = nullptr;
	
	FFluxPrimeCrowds* crowdsDatas = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsDataModule
{
	GENERATED_BODY()
	
private:
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* CrowdsComponents = nullptr;
	TArray<FFluxPrimeCrowdsCatalog>* CrowdsCatalog = nullptr;
	
	TArray<TArray<int16>>* CrowdsPool = nullptr;
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>>* CrowdsAnimationSoftRef = nullptr;
	TMap<FFluxPrimeCrowdsLookup, int32>* CrowdsLookup = nullptr;
	
	uint16* CrowdsTotal = nullptr;
	
	FFluxPrimeCrowds* CrowdsDatas = nullptr;
	
public:
	void Initialize(FFluxPrimeCrowdsDataModuleContext context)
	{
		check(context.crowdsCatalog);
		check(context.crowdsDatas);
		check(context.crowdsPool);
		check(context.crowdsAnimationSoftRef);
		check(context.crowdsComponents);
		check(context.crowdsLookup);
		
		CrowdsComponents = context.crowdsComponents;
		CrowdsCatalog = context.crowdsCatalog;
		CrowdsPool = context.crowdsPool;
		CrowdsAnimationSoftRef = context.crowdsAnimationSoftRef;
		CrowdsLookup = context.crowdsLookup;
		CrowdsDatas = context.crowdsDatas;
		CrowdsTotal = context.crowdsTotal;
	}
	
	// perlu refactor
	void InitializeCrowds()
	{
		for (int i = 0; i < (*CrowdsCatalog).Num(); ++i) *CrowdsTotal += (*CrowdsCatalog)[i].CrowdsTotal;
		
		CrowdsDatas->Init(*CrowdsTotal);
		
		auto& members = *CrowdsDatas;
		auto& catalog = *CrowdsCatalog;
		auto& pool = *CrowdsPool;
		auto& lookUp = *CrowdsLookup;
		
		int32 indexData = 0;
		for (int i = 0; i < catalog.Num(); ++i)
		{
			UFluxPrimeCrowdsIdentity& indentity = *catalog[i].CrowdsIdentity;
			
			for (int j = 0; j < catalog[i].CrowdsTotal; ++j)
			{
				FTransform tempTransform;
				tempTransform.SetLocation(FVector::DownVector * 1000.0f);
				int32 id = (*CrowdsComponents)[i]->AddInstance(tempTransform, false);
				
				members.CrowdsLocation.Add(tempTransform.GetLocation());
				members.CrowdsRotation.Add(0);
				members.CrowdsVelocity.Add(FVector::ZeroVector);
				members.CrowdsAcceleration.Add(FVector::ZeroVector);
				members.CrowdsID.Add(id);
				members.CrowdsType.Add(i);
				members.CrowdsCellID.Add(-1);
				members.CrowdsCondition.Add(false);
				members.CrowdsRequestBackToPool.Add(false);
				members.CrowdsMaxSpeed.Add(indentity.Speed);
				members.CrowdsHealth.Add(indentity.Health);
				members.CrowdsDamage.Add(indentity.Damage);
				members.CrowdsAbilityRange.Add(indentity.AbilityRange);
				members.CrowdsRequestAbility.Add(false);
				members.CrowdsSize.Add(indentity.Size);
				members.CrowdsState.Add(EFluxPrimeCrowdState::StateIdle);
				members.CrowdsAnimationState.Add(EFluxPrimeCrowdState::StateIdle);
				members.CrowdsRequestAnimationNotify.Add(EFluxPrimeCrowdAnimationNotify::NotifyNone);
				members.CrowdsStartTimeAnimation.Add(0);
				members.CrowdsPreviousAnimationFrame.Add(-1.0f);
				
				members.CrowdsPreviousLocation.Add(FVector::ZeroVector);
				members.CrowdsCorridors.Add(FFluxPrimeCrowdsCorridor());
				members.CrowdsTargetID.Add(-1);
				members.CrowdsTarget.Add(FVector::ZeroVector);
				members.CrowdsCurrentTarget.Add(FVector::ZeroVector);
				members.CrowdsLastReplanTarget.Add(FVector::ZeroVector);
				members.CrowdsLastMoveTarget.Add(FVector::ZeroVector);
				members.CrowdsLastOptimizeTime.Add(0.0f);
				members.CrowdsLastMoveTargetTime.Add(0.0f);
				members.CrowdsCountCorridor.Add(0);
				members.CrowdsRequestNeedReplan.Add(false);
				members.CrowdsWaypoints.Add(FFluxPrimeCrowdsWaypoint());
				members.CrowdsCountWaypoints.Add(0);
				
				pool[i].Add(indexData);
				
				FFluxPrimeCrowdsLookup lookup;
				lookup.CrowdsID = id;
				lookup.CrowdsType = i;
				lookUp.Add(lookup, j);
				
				++indexData;
			}
		}
	}
};
