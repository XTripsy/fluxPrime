#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "Framework/Data/FluxPrimeStateData.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "FluxPrimeBaseStateMachineSystem.generated.h"

USTRUCT(BlueprintType)
struct FStateMachineSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataStateTag;


	bool IsValid() const
	{
		return EntityDataStateTag.IsValid();
	}
};

using FStateExecute = void(*)(FInstancedStruct&, float, const int32, const int32, FFluxPrimeArchetypeDataStore&);
using FStateTransitionExecute = void(*)(FInstancedStruct&, int8&, const int32, const int32, FFluxPrimeArchetypeDataStore&);

struct FStateExecutor
{
	FInstancedStruct* Data = nullptr;
	FStateExecute Execute = nullptr;
};

struct FStateTransitionExecutor
{
	FInstancedStruct* Data = nullptr;
	FStateTransitionExecute Execute = nullptr;
};

USTRUCT()
struct FFluxPrimeRuntimeDataStateMapping
{
	GENERATED_BODY()
	
	TArray<FStateExecutor> State;
	TArray<FStateTransitionExecutor> Transition;
};

USTRUCT(BlueprintType)
struct FFluxPrimeBaseStateMachineSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
protected:
	TArray<FFluxPrimeRuntimeDataStateMapping> RuntimeDataStateMachine;
	
public:
	UPROPERTY(EditAnywhere)
	FStateMachineSystemDataTagConfig DataTagConfig;
	
	TArray<int16> EntityDataStateID;
	
private:
	template<typename T>
	static void ExecuteState(FInstancedStruct& dataState, float deltaTime, const int32 indexDataID,const int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		T& state = dataState.GetMutable<T>();
		state.ExecuteState(deltaTime, indexDataID, indexEntity, dataStore);
	}
	
	template<typename T>
	static void ExecuteTransition(FInstancedStruct& dataTransition, int8& currentState, const int32 indexDataID, const int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		T& transition = dataTransition.GetMutable<T>();
		transition.ExecuteStateTransition(currentState, indexDataID, indexEntity, dataStore);
	}
	
protected:
	template<typename TState>
	int32 RegisterStateRuntimeData(FInstancedStruct& sourceState, int32 indexData)
	{
		if (indexData == INDEX_NONE) return INDEX_NONE;
		
		FStateExecutor executorState;
		executorState.Data = &sourceState;
		executorState.Execute = &ExecuteState<TState>;
		
		return RuntimeDataStateMachine[indexData].State.Add(MoveTemp(executorState));
	}
	
	template<typename TTransition>
	int32 RegisterTransitionRuntimeData(FInstancedStruct& sourceTransition, int32 indexData)
	{
		if (indexData == INDEX_NONE) return INDEX_NONE;
		
		FStateTransitionExecutor executorTransition;
		executorTransition.Data = &sourceTransition;
		executorTransition.Execute = &ExecuteTransition<TTransition>;
		
		return RuntimeDataStateMachine[indexData].Transition.Add(MoveTemp(executorTransition));
	}
	
public:
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_FSM_Systems);
		
		if (RuntimeDataStateMachine.IsEmpty()) return;

		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			FFluxPrimeWrapperInt8* entityDataState = reinterpret_cast<FFluxPrimeWrapperInt8*>(dataStore.GetRawBufferData(EntityDataStateID[indexDataID]));
			
			int8 currentState = -1;
			for (int indexEntity = 0; indexEntity < dataStore.GetActiveEntityCount(); ++indexEntity)
			{
				currentState = entityDataState[indexEntity].ValueInt8;
				FFluxPrimeRuntimeDataStateMapping& pairData = RuntimeDataStateMachine[indexDataID];
				
				FStateExecutor& pairState = pairData.State[currentState];
				FStateTransitionExecutor& pairTransition = pairData.Transition[currentState];
				
				pairState.Execute(*pairState.Data, deltaTime, indexDataID, indexEntity, dataStore);
				pairTransition.Execute(*pairTransition.Data, entityDataState[indexEntity].ValueInt8, indexDataID, indexEntity, dataStore);
			}
			
			indexDataID++;
		}
	}
};
