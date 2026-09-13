#pragma once

#include "CoreMinimal.h"
#include "TransitionLogics/FluxPrimeCrowdsDistanceTransition.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Systems/StateMachine/FluxPrimeBaseStateTransition.h"
#include "TransitionLogics/FluxPrimeCrowdsDeadTransition.h"
#include "FluxPrimeCrowdsStateTransition.generated.h"

USTRUCT()
struct FFluxPrimeCrowdsStateTransitionContext
{
	GENERATED_BODY()
	
	FGetStateID getStateID = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsStateTransition : public FFluxPrimeBaseStateTransition
{
	GENERATED_BODY()
	
private:
	void RegisterTransitions(TArray<FFluxPrimeArchetypeDataStore>& dataStores, const FGetStateID getStateID)
	{
		for (auto& pair : TransitionFragments)
		{
			if (pair.TransitionFragment.GetPtr<FFluxPrimeCrowdsDistanceTransition>())
			{
				const int32 currentIndex = RegisterTransition<FFluxPrimeCrowdsDistanceTransition>(getStateID, pair.TransitionFragment, pair.FromStateTag, pair.ToStateTag);
				
				FFluxPrimeStateTransitionExecutor& Executor = Transition[currentIndex];
				FFluxPrimeCrowdsDistanceTransition& transition = Executor.Data->GetMutable<FFluxPrimeCrowdsDistanceTransition>();
				transition.Initialized(dataStores);
			}
			
			if (pair.TransitionFragment.GetPtr<FFluxPrimeCrowdsDeadTransition>())
			{
				const int32 currentIndex = RegisterTransition<FFluxPrimeCrowdsDeadTransition>(getStateID, pair.TransitionFragment, pair.FromStateTag, pair.ToStateTag);
				
				FFluxPrimeStateTransitionExecutor& Executor = Transition[currentIndex];
				FFluxPrimeCrowdsDeadTransition& transition = Executor.Data->GetMutable<FFluxPrimeCrowdsDeadTransition>();
				transition.Initialized(dataStores);
			}
		}
	}
	
public:
	void Initialized(FFluxPrimeCrowdsStateTransitionContext context)
	{
		check(context.getStateID);
		check(context.dataStores);
		
		RegisterTransitions(*context.dataStores, context.getStateID);
	}
};
