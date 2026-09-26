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

	UPROPERTY()
	TObjectPtr<UWorld> world = nullptr;
	
	FGetNotifyID getNotifyID = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrwodsAnimationNotifySystem : public FFluxPrimeBaseAnimationNotifySystem
{
	GENERATED_BODY()
	
private:
	void RegisterAbilityExecutors(TArray<FFluxPrimeArchetypeDataStore>& dataStores, const TObjectPtr<UWorld> world, const FGetNotifyID& getNotifyID)
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
				context.dataStores = &dataStores;

				FNotifyExecutor& Executor = NotifyExecutors[executorIndex];
				FFluxPrimeCrowdsNotifyDead& ability = Executor.Data->GetMutable<FFluxPrimeCrowdsNotifyDead>();
				ability.Initialized(context);
			}
			
			if (NotifyFragments[i].GetPtr<FFluxPrimeCrowdsNotifySpawnSFX>())
			{
				const int32 executorIndex = RegisterAbility<FFluxPrimeCrowdsNotifySpawnSFX>(NotifyFragments[i]);
				
				FFluxPrimeCrowdsNotifySpawnSFXContext context;
				context.world = world;
				context.getNotifyID = getNotifyID;
				context.dataStores = &dataStores;

				FNotifyExecutor& Executor = NotifyExecutors[executorIndex];
				FFluxPrimeCrowdsNotifySpawnSFX& ability = Executor.Data->GetMutable<FFluxPrimeCrowdsNotifySpawnSFX>();
				ability.Initialized(context);
			}
			
			if (NotifyFragments[i].GetPtr<FFluxPrimeCrowdsNotifySpawnVFX>())
			{
				const int32 executorIndex = RegisterAbility<FFluxPrimeCrowdsNotifySpawnVFX>(NotifyFragments[i]);
				
				FFluxPrimeCrowdsNotifySpawnVFXContext context;
				context.world = world;
				context.getNotifyID = getNotifyID;
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
		check(contex.world);
		check(contex.getNotifyID);
		check(contex.dataStores);
		
		RegisterAbilityExecutors(*contex.dataStores, contex.world, contex.getNotifyID);
	}
};