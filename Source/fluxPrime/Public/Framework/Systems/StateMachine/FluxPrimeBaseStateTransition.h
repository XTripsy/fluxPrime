#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Framework/Data/FluxPrimeStateData.h"
#include "FluxPrimeBaseStateTransition.generated.h"

using FFluxPrimeTransitionConditionExecute = bool (*)(FInstancedStruct&, const int32, const int32, FFluxPrimeArchetypeDataStore&);

struct FFluxPrimeStateTransitionExecutor
{
	FInstancedStruct* Data = nullptr;
	FFluxPrimeTransitionConditionExecute ConditionExecute = nullptr;

	int8 FromState;
	int8 NextState;
};

USTRUCT()
struct FFluxPrimeTransitionFragment
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FGameplayTag FromStateTag;
	
	UPROPERTY(EditAnywhere)
	FGameplayTag ToStateTag;
	
	UPROPERTY(EditAnywhere, meta=(BaseStruct="FluxPrimeBaseTransition", ExcludeBaseStruct))
	FInstancedStruct TransitionFragment;
};

USTRUCT(BlueprintType)
struct FFluxPrimeBaseStateTransition
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	TArray<FFluxPrimeTransitionFragment> TransitionFragments;
	
	TArray<FFluxPrimeStateTransitionExecutor> Transition;
		
private:
	template<typename T>
	static bool ExecuteTransition(FInstancedStruct& data, const int32 indexDataID, const int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		T& transition = data.GetMutable<T>();
		return transition.ExecuteTransition(indexDataID, indexEntity, dataStore);
	}
	
	void ChangeState(int8& fromState, const int8& newState)
	{
		fromState = newState;
	}
	
protected:
	template<typename T>
	int32 RegisterTransition(const FGetStateID getStateID, FInstancedStruct& source, FGameplayTag fromState, FGameplayTag newState)
	{
		check(getStateID);
		
		int32 idFromState = getStateID(fromState);
		int32 idNewState = getStateID(newState);
		
		check(idFromState != INDEX_NONE);
		check(idNewState != INDEX_NONE);
		
		FFluxPrimeStateTransitionExecutor executor;
		executor.FromState = idFromState;
		executor.NextState = idNewState;
		executor.Data = &source;
		executor.ConditionExecute = &ExecuteTransition<T>;

		return Transition.Add(MoveTemp(executor));
	}
	
public:
	void ExecuteStateTransition(int8& currentState,  int32 indexDataID, int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		for (auto& pair : Transition)
		{
			if (currentState == pair.FromState && pair.ConditionExecute(*pair.Data, indexDataID, indexEntity, dataStore))
				ChangeState(currentState, pair.NextState);
		}
	}
};
