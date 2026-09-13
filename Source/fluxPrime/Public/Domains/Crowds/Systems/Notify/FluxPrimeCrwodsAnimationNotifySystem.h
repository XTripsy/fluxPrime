#pragma once

#include "CoreMinimal.h"
#include "DrawDebugHelpers.h"
#include "Generics/Systems/Animation/FluxPrimeBaseAnimationNotifySystem.h"
#include "FluxPrimeCrowdsNotifyAbility.h"
#include "FluxPrimeCrowdsNotifyDead.h"
#include "FluxPrimeCrowdsNotifySpawnSFX.h"
#include "FluxPrimeCrowdsNotifySpawnVFX.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeCrwodsAnimationNotifySystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationNotifySystemsContext
{
	GENERATED_BODY()

	FGetNotifyID getNotifyID = nullptr;
	//FFluxPrimeDataStore* dataStore = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrwodsAnimationNotifySystem : public FFluxPrimeBaseAnimationNotifySystem
{
	GENERATED_BODY()
	
private:
	//void RegisterAbilityExecutors(FFluxPrimeDataStore& dataStore, const FGetNotifyID& getNotifyID)
	void RegisterAbilityExecutors(TArray<FFluxPrimeArchetypeDataStore>& dataStores, const FGetNotifyID& getNotifyID)
	{
		NotifyExecutors.Reset();
		NotifyExecutors.Reserve(NotifyFragments.Num());
		
		for (int i = 0; i < NotifyFragments.Num(); ++i)
		{
			if (NotifyFragments[i].GetPtr<FFluxPrimeCrowdsNotifyAbility>())
			{
				const int32 executorIndex = RegisterAbility<FFluxPrimeCrowdsNotifyAbility>(NotifyFragments[i]);
				
				FFluxPrimeCrowdsNotifyAbilityContext context;
				context.getNotifyID = getNotifyID;
				//context.dataStore = &dataStore;
				context.dataStores = &dataStores;

				FNotifyExecutor& Executor = NotifyExecutors[executorIndex];
				FFluxPrimeCrowdsNotifyAbility& ability = Executor.Data->GetMutable<FFluxPrimeCrowdsNotifyAbility>();
				ability.Initialized(context);
			}
			
			if (NotifyFragments[i].GetPtr<FFluxPrimeCrowdsNotifyDead>())
			{
				const int32 executorIndex = RegisterAbility<FFluxPrimeCrowdsNotifyDead>(NotifyFragments[i]);
				
				FFluxPrimeCrowdsNotifyDeadContext context;
				context.getNotifyID = getNotifyID;
				//context.dataStore = &dataStore;
				context.dataStores = &dataStores;

				FNotifyExecutor& Executor = NotifyExecutors[executorIndex];
				FFluxPrimeCrowdsNotifyDead& ability = Executor.Data->GetMutable<FFluxPrimeCrowdsNotifyDead>();
				ability.Initialized(context);
			}
			
			if (NotifyFragments[i].GetPtr<FFluxPrimeCrowdsNotifySpawnSFX>())
			{
				const int32 executorIndex = RegisterAbility<FFluxPrimeCrowdsNotifySpawnSFX>(NotifyFragments[i]);
				
				FFluxPrimeCrowdsNotifySpawnSFXContext context;
				context.getNotifyID = getNotifyID;
				//context.dataStore = &dataStore;
				context.dataStores = &dataStores;

				FNotifyExecutor& Executor = NotifyExecutors[executorIndex];
				FFluxPrimeCrowdsNotifySpawnSFX& ability = Executor.Data->GetMutable<FFluxPrimeCrowdsNotifySpawnSFX>();
				ability.Initialized(context);
			}
			
			if (NotifyFragments[i].GetPtr<FFluxPrimeCrowdsNotifySpawnVFX>())
			{
				const int32 executorIndex = RegisterAbility<FFluxPrimeCrowdsNotifySpawnVFX>(NotifyFragments[i]);
				
				FFluxPrimeCrowdsNotifySpawnVFXContext context;
				context.getNotifyID = getNotifyID;
				//context.dataStore = &dataStore;
				context.dataStores = &dataStores;

				FNotifyExecutor& Executor = NotifyExecutors[executorIndex];
				FFluxPrimeCrowdsNotifySpawnVFX& ability = Executor.Data->GetMutable<FFluxPrimeCrowdsNotifySpawnVFX>();
				ability.Initialized(context);
			}
		}
	}
	
public:
	void Initialized(FFluxPrimeAnimationNotifySystemsContext contex)
	{
		check(contex.getNotifyID);
		//check(contex.dataStore);
		check(contex.dataStores);
		
		//RegisterAbilityExecutors(*contex.dataStore, contex.getNotifyID);
		RegisterAbilityExecutors(*contex.dataStores, contex.getNotifyID);
	}
};

/*USTRUCT(BlueprintType)
struct FFluxPrimeAnimationNotifySystemsContext
{
	GENERATED_BODY()
	
	FFluxPrimeCrowds* crowdsDatas = nullptr;
	uint16* memberActive = nullptr;
	FGetNotifyID getNotifyID = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrwodsAnimationNotifySystem : public FFluxPrimeBaseAnimationNotifySystem
{
	GENERATED_BODY()
	
private:
	FFluxPrimeCrowds* CrowdsDatas = nullptr;
	
private:
	void RegisterAbilityExecutors()
	{
		auto& members = *CrowdsDatas;
		
		NotifyExecutors.Reset();
		NotifyExecutors.Reserve(NotifyFragments.Num());
		
		for (int i = 0; i < NotifyFragments.Num(); ++i)
		{
			if (NotifyFragments[i].GetPtr<FFluxPrimeCrowdsNotifyAbility>())
			{
				const int32 executorIndex = RegisterAbility<FFluxPrimeCrowdsNotifyAbility>(NotifyFragments[i]);
				
				FFluxPrimeCrowdsNotifyAbilityContext context;
				context.requestAbilityCrowds = &members.CrowdsRequestAbility;
				context.requestNotifyCrowds = &members.CrowdsRequestAnimationNotify; 
				context.getNotifyID = GetNotifyID;

				FNotifyExecutor& Executor = NotifyExecutors[executorIndex];
				FFluxPrimeCrowdsNotifyAbility& ability = Executor.Data->GetMutable<FFluxPrimeCrowdsNotifyAbility>();
				ability.Initialized(context);
				continue;
			}
			
			if (NotifyFragments[i].GetPtr<FFluxPrimeCrowdsNotifyDead>())
			{
				const int32 executorIndex = RegisterAbility<FFluxPrimeCrowdsNotifyDead>(NotifyFragments[i]);
				
				FFluxPrimeCrowdsNotifyDeadContext context;
				context.requestNotifyCrowds = &members.CrowdsRequestAnimationNotify; 
				context.requestBackPoolCrowds = &members.CrowdsRequestBackToPool;
				context.locationCrowds = &members.CrowdsLocation;
				context.getNotifyID = GetNotifyID;

				FNotifyExecutor& Executor = NotifyExecutors[executorIndex];
				FFluxPrimeCrowdsNotifyDead& ability = Executor.Data->GetMutable<FFluxPrimeCrowdsNotifyDead>();
				ability.Initialized(context);
				continue;
			}
			
			if (NotifyFragments[i].GetPtr<FFluxPrimeCrowdsNotifySpawnSFX>())
			{
				const int32 executorIndex = RegisterAbility<FFluxPrimeCrowdsNotifySpawnSFX>(NotifyFragments[i]);
				
				FFluxPrimeCrowdsNotifySpawnSFXContext context;
				context.requestNotifyCrowds = &members.CrowdsRequestAnimationNotify; 
				context.getNotifyID = GetNotifyID;

				FNotifyExecutor& Executor = NotifyExecutors[executorIndex];
				FFluxPrimeCrowdsNotifySpawnSFX& ability = Executor.Data->GetMutable<FFluxPrimeCrowdsNotifySpawnSFX>();
				ability.Initialized(context);
				continue;
			}
			
			if (NotifyFragments[i].GetPtr<FFluxPrimeCrowdsNotifySpawnVFX>())
			{
				const int32 executorIndex = RegisterAbility<FFluxPrimeCrowdsNotifySpawnVFX>(NotifyFragments[i]);
				
				FFluxPrimeCrowdsNotifySpawnVFXContext context;
				context.requestNotifyCrowds = &members.CrowdsRequestAnimationNotify; 
				context.getNotifyID = GetNotifyID;

				FNotifyExecutor& Executor = NotifyExecutors[executorIndex];
				FFluxPrimeCrowdsNotifySpawnVFX& ability = Executor.Data->GetMutable<FFluxPrimeCrowdsNotifySpawnVFX>();
				ability.Initialized(context);
				continue;
			}
		}
	}
	
public:
	void Initialized(FFluxPrimeAnimationNotifySystemsContext contex)
	{
		check(contex.crowdsDatas);
		check(contex.memberActive);
		check(contex.getNotifyID);
		
		CrowdsDatas = contex.crowdsDatas;
		MemberActive = contex.memberActive;
		GetNotifyID = contex.getNotifyID;
		
		RegisterAbilityExecutors();
	}
};*/