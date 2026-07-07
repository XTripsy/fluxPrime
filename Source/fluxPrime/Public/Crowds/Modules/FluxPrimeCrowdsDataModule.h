#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Cores/FluxPrimeAnimationData.h"
#include "Cores/FluxPrimeStruct.h"
#include "Crowds/Identity/CrowdsIdentity.h"
#include "FluxPrimeCrowdsDataModule.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsDataComponentContext
{
	GENERATED_BODY()
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* crowdsComponents = nullptr;
	TArray<FFluxPrimeCrowdsCatalog>* crowdsCatalog;
	
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>>* crowdsAnimationSoftRef = nullptr;
	
	TMap<FFluxPrimeCrowdsLookup, int32>* crowdsLookup = nullptr;
	
	uint16* crowdsTotal = nullptr;
	//int8* crowdsDataReadIndex = nullptr;
	
	//TArray<int32>* crowdsDataShortedIndex = nullptr;
	
	TArray<FVector_NetQuantize100>* netAcceleration = nullptr;
	TArray<FVector_NetQuantize100>* netTarget = nullptr;
	
	//TStaticArray<FFluxPrimeCrowds, 2>* crowdsDatas = nullptr;
	FFluxPrimeCrowds* crowdsDatas = nullptr;
	
	bool hasAuthority, isReplicated;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsDataModule
{
	GENERATED_BODY()
	
private:
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* CrowdsComponents = nullptr;
	TArray<FFluxPrimeCrowdsCatalog>* CrowdsCatalog = nullptr;
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>>* CrowdsAnimationSoftRef = nullptr;
	TMap<FFluxPrimeCrowdsLookup, int32>* CrowdsLookup = nullptr;
	
	uint16* CrowdsTotal = nullptr;
	//int8* CrowdsDataReadIndex = nullptr;
	
	//TArray<int32>* CrowdsDataShortedIndex = nullptr;
	TArray<FVector_NetQuantize100>* NetAcceleration = nullptr;
	TArray<FVector_NetQuantize100>* NetTarget = nullptr;
	
	//TStaticArray<FFluxPrimeCrowds, 2>* CrowdsDatas = nullptr;
	FFluxPrimeCrowds* CrowdsDatas = nullptr;
	
	bool bHasAuthority = false, bIsReplicated = false;
	
public:
	void Initialize(FFluxPrimeCrowdsDataComponentContext context)
	{
		check(context.crowdsCatalog);
		check(context.crowdsDatas);
		check(context.crowdsAnimationSoftRef);
		check(context.crowdsComponents);
		check(context.crowdsLookup);
		
		CrowdsComponents = context.crowdsComponents;
		CrowdsCatalog = context.crowdsCatalog;
		CrowdsAnimationSoftRef = context.crowdsAnimationSoftRef;
		CrowdsLookup = context.crowdsLookup;
		//CrowdsDataReadIndex = context.crowdsDataReadIndex;
		//CrowdsDataShortedIndex = context.crowdsDataShortedIndex;
		NetAcceleration = context.netAcceleration;
		NetTarget = context.netTarget;
		CrowdsDatas = context.crowdsDatas;
		CrowdsTotal = context.crowdsTotal;
		bHasAuthority = context.hasAuthority;
		bIsReplicated = context.isReplicated;
	}
	
	void InitializeCrowds()
	{
		for (int i = 0; i < (*CrowdsCatalog).Num(); ++i) *CrowdsTotal += (*CrowdsCatalog)[i].CrowdsTotal;
		
		/*auto& crowdRead = (*CrowdsDatas)[*CrowdsDataReadIndex];
		auto& crowdWrite = (*CrowdsDatas)[*CrowdsDataReadIndex+1];*/
		
		/*crowdRead.Init(*CrowdsTotal);
		crowdWrite.Init(*CrowdsTotal);*/
		//CrowdsDataShortedIndex->SetNumUninitialized(*CrowdsTotal);
		NetAcceleration->Init(FVector(), *CrowdsTotal);
		NetTarget->Init(FVector(), *CrowdsTotal);
		
		auto& members = *CrowdsDatas;
		auto& catalog = *CrowdsCatalog;
		auto& lookUp = *CrowdsLookup;
		
		for (int i = 0; i < catalog.Num(); ++i)
		{
			UCrowdsIdentity& indentity = *catalog[i].CrowdsIdentity;
			
			for (int j = 0; j < catalog[i].CrowdsTotal; ++j)
			{
				FTransform tempTransform;
				tempTransform.SetLocation(FVector::DownVector * 1000.0f);
				int32 id = (*CrowdsComponents)[i]->AddInstance(tempTransform, false);
				
				if (!bHasAuthority && bIsReplicated) continue;
				
				FFluxPrimeCrowdsAnimation animationData = FFluxPrimeCrowdsAnimation();

				TSoftObjectPtr<UFluxPrimeAnimationData> loadedData = (*CrowdsAnimationSoftRef)[catalog[i].CrowdsIdentity->Identity];
				UFluxPrimeAnimationData& dataAnim = *loadedData;
				
				for (int k = 0; k < dataAnim.DataAnimations.Num(); ++k)
				{
					int32 index = static_cast<uint8>(dataAnim.DataAnimations[k].AnimationState);
					FFluxPrimeCrowdsAnimationMapping mapping = FFluxPrimeCrowdsAnimationMapping();
					mapping.AnimationStart = dataAnim.DataAnimations[k].AnimationStartFrame;
					mapping.AnimationEnd = dataAnim.DataAnimations[k].AnimationEndFrame;
					mapping.AnimationLoop = dataAnim.DataAnimations[k].AnimationLoops;
					for (int32 l = 0; l < FluxConfig::AnimationArrayCount; ++l) mapping.AnimationNotify[l] = dataAnim.DataAnimations[k].AnimationNotifies[l];
					if (index  < 0 || index > animationData.AnimationData.Num()) continue;
					
					animationData.AnimationData[index] = mapping;
				}
				
				/*crowdRead.CrowdsLocation.Add(tempTransform.GetLocation());
				crowdRead.CrowdsRotation.Add(0);
				crowdRead.CrowdsVelocity.Add(FVector::ZeroVector);
				crowdRead.CrowdsAcceleration.Add(FVector::ZeroVector);
				crowdRead.CrowdsID.Add(id);
				crowdRead.CrowdsType.Add(i);
				crowdRead.CrowdsCellID.Add(-1);
				crowdRead.CrowdsMaxSpeed.Add(((*CrowdsCatalog))[i].CrowdsIdentity->Speed);
				crowdRead.CrowdsHealth.Add(((*CrowdsCatalog))[i].CrowdsIdentity->Health);
				crowdRead.CrowdsDamage.Add((*CrowdsCatalog)[i].CrowdsIdentity->Damage);
				crowdRead.CrowdsSize.Add((*CrowdsCatalog)[i].CrowdsIdentity->Size);
				crowdRead.CrowdsState.Add(EFluxPrimeCrowdState::StateIdle);
				crowdRead.CrowdsNavigationPath.Add(FFluxPrimeCrowdsPath());
				crowdRead.CrowdsTargetLocation.Add(FVector::ZeroVector);
				//(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsCurrentTargetLocationPath.Add(FVector::ZeroVector);
				crowdRead.CrowdsRequestNavigationPath.Add(false);
				crowdRead.CrowdsIndexNavigationPath.Add(0);
				crowdRead.CrowdsTotalNavigationPath.Add(0);
				crowdRead.CrowdsAnimationMapping.Add(animationData);
				crowdRead.CrowdsAnimationState.Add(EFluxPrimeCrowdState::StateIdle);
				crowdRead.CrowdsStartTimeAnimation.Add(0);
				crowdRead.CrowdsPreviousAnimationFrame.Add(-1.0f);*/
				
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
				members.CrowdsSize.Add(indentity.Size);
				members.CrowdsState.Add(EFluxPrimeCrowdState::StateIdle);
				members.CrowdsNavigationPath.Add(FFluxPrimeCrowdsPath());
				members.CrowdsTargetLocation.Add(FVector::ZeroVector);
				//(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsCurrentTargetLocationPath.Add(FVector::ZeroVector);
				members.CrowdsRequestNavigationPath.Add(false);
				members.CrowdsIndexNavigationPath.Add(0);
				members.CrowdsTotalNavigationPath.Add(0);
				members.CrowdsAnimationMapping.Add(animationData);
				members.CrowdsAnimationState.Add(EFluxPrimeCrowdState::StateIdle);
				members.CrowdsStartTimeAnimation.Add(0);
				members.CrowdsPreviousAnimationFrame.Add(-1.0f);
				
				FFluxPrimeCrowdsLookup lookup;
				lookup.CrowdsID = id;
				lookup.CrowdsType = i;
				lookUp.Add(lookup, j);
			}
		}
		
		//crowdWrite = crowdRead;
	}
};
