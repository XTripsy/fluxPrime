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
	
	uint16* crowdsTotal = nullptr;
	int8* crowdsDataReadIndex = nullptr;
	
	TArray<int32>* crowdsDataShortedIndex = nullptr;
	
	TArray<FVector_NetQuantize100>* netAcceleration = nullptr;
	TArray<FVector_NetQuantize100>* netTarget = nullptr;
	
	TStaticArray<FFluxPrimeCrowds, 2>* crowdsDatas = nullptr;
	
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
	
	uint16* CrowdsTotal = nullptr;
	int8* CrowdsDataReadIndex = nullptr;
	
	TArray<int32>* CrowdsDataShortedIndex = nullptr;
	TArray<FVector_NetQuantize100>* NetAcceleration = nullptr;
	TArray<FVector_NetQuantize100>* NetTarget = nullptr;
	
	TStaticArray<FFluxPrimeCrowds, 2>* CrowdsDatas = nullptr;
	
	bool bHasAuthority = false, bIsReplicated = false;
	
public:
	void Initialize(FFluxPrimeCrowdsDataComponentContext context)
	{
		check(context.crowdsCatalog);
		check(context.crowdsDatas);
		check(context.crowdsAnimationSoftRef);
		check(context.crowdsComponents);
		
		CrowdsComponents = context.crowdsComponents;
		CrowdsCatalog = context.crowdsCatalog;
		CrowdsAnimationSoftRef = context.crowdsAnimationSoftRef;
		CrowdsDataReadIndex = context.crowdsDataReadIndex;
		CrowdsDataShortedIndex = context.crowdsDataShortedIndex;
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
		
		(*CrowdsDatas)[*CrowdsDataReadIndex].Init(*CrowdsTotal);
		(*CrowdsDatas)[*CrowdsDataReadIndex+1].Init(*CrowdsTotal);
		CrowdsDataShortedIndex->SetNumUninitialized(*CrowdsTotal);
		(*NetAcceleration).Init(FVector(), *CrowdsTotal);
		(*NetTarget).Init(FVector(), *CrowdsTotal);
		
		for (int i = 0; i < (*CrowdsCatalog).Num(); ++i)
		{
			for (int j = 0; j < (*CrowdsCatalog)[i].CrowdsTotal; ++j)
			{
				FTransform tempTransform;
				tempTransform.SetLocation(FVector::DownVector * 1000.0f);
				int32 id = (*CrowdsComponents)[i]->AddInstance(tempTransform, false);
				
				if (!bHasAuthority && bIsReplicated) continue;
				
				FFluxPrimeCrowdsAnimation animationData = FFluxPrimeCrowdsAnimation();

				TSoftObjectPtr<UFluxPrimeAnimationData> loadedData = (*CrowdsAnimationSoftRef)[(*CrowdsCatalog)[i].CrowdsIdentity->Identity];
				
				for (int k = 0; k < loadedData->DataAnimations.Num(); ++k)
				{
					int32 index = static_cast<uint8>(loadedData->DataAnimations[k].AnimationState);
					FFluxPrimeCrowdsAnimationMapping mapping = FFluxPrimeCrowdsAnimationMapping();
					mapping.AnimationStart = loadedData->DataAnimations[k].AnimationStartFrame;
					mapping.AnimationEnd = loadedData->DataAnimations[k].AnimationEndFrame;
					mapping.AnimationLoop = loadedData->DataAnimations[k].AnimationLoops;
					for (int32 l = 0; l < FluxConfig::AnimationArrayCount; ++l) mapping.AnimationNotify[l] = loadedData->DataAnimations[k].AnimationNotifies[l];
					if (index  < 0 || index > animationData.AnimationData.Num()) continue;
					
					animationData.AnimationData[index] = mapping;
				}
				
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsLocation.Add(tempTransform.GetLocation());
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsRotation.Add(0);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsVelocity.Add(FVector::ZeroVector);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsAcceleration.Add(FVector::ZeroVector);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsID.Add(id);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsType.Add(i);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsCellID.Add(-1);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsMaxSpeed.Add(((*CrowdsCatalog))[i].CrowdsIdentity->Speed);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsHealth.Add(((*CrowdsCatalog))[i].CrowdsIdentity->Health);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsDamage.Add((*CrowdsCatalog)[i].CrowdsIdentity->Damage);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsSize.Add((*CrowdsCatalog)[i].CrowdsIdentity->Size);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsState.Add(EFluxPrimeCrowdState::StateIdle);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsNavigationPath.Add(FFluxPrimeCrowdsPath());
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsTargetLocation.Add(FVector::ZeroVector);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsCurrentTargetLocationPath.Add(FVector::ZeroVector);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsIndexNavigationPath.Add(0);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsTotalNavigationPath.Add(0);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsAnimationMapping.Add(animationData);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsAnimationState.Add(EFluxPrimeCrowdState::StateIdle);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsStartTimeAnimation.Add(0);
				(*CrowdsDatas)[*CrowdsDataReadIndex].CrowdsPreviousAnimationFrame.Add(-1.0f);
			}
		}
		
		(*CrowdsDatas)[*CrowdsDataReadIndex+1] = (*CrowdsDatas)[*CrowdsDataReadIndex];
	}
};
