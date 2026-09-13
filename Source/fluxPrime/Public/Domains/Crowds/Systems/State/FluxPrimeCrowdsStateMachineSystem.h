#pragma once

#include "CoreMinimal.h"
#include "StateLogics/FluxPrimeCrowdsWalkState.h"
#include "FluxPrimeCrowdsStateTransition.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsData.h"
#include "Framework/Data/FluxPrimeStateData.h"
#include "Framework/Systems/StateMachine/FluxPrimeBaseStateMachineSystem.h"
#include "StateLogics/FluxPrimeCrowdsAbilityState.h"
#include "StateLogics/FluxPrimeCrowdsDeadState.h"
#include "StateLogics/FluxPrimeCrowdsIdleState.h"
#include "FluxPrimeCrowdsStateMachineSystem.generated.h"

USTRUCT()
struct FFluxPrimeCrowdsStateMachineSystemContext
{
	GENERATED_BODY()
	
	FGetStateID getStateID = nullptr;
	TArray<FFluxPrimeArchetypeEntityCatalog>* entityCatalog = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsStateMachineSystem : public FFluxPrimeBaseStateMachineSystem
{
	GENERATED_BODY()
	
private:
	void InitializedStateData(TArray<FFluxPrimeArchetypeDataStore>& dataStores, int32 indexData, FInstancedStruct& sourceState)
	{
		if (sourceState.GetPtr<FFluxPrimeCrowdsWalkState>())
		{
			const int32 currentIndex = RegisterStateRuntimeData<FFluxPrimeCrowdsWalkState>(sourceState, indexData);
		}
		
		if (sourceState.GetPtr<FFluxPrimeCrowdsAbilityState>())
		{
			const int32 currentIndex = RegisterStateRuntimeData<FFluxPrimeCrowdsAbilityState>(sourceState, indexData);
			
			FStateExecutor& Executor = RuntimeDataStateMachine[indexData].State[currentIndex];
			FFluxPrimeCrowdsAbilityState& state = Executor.Data->GetMutable<FFluxPrimeCrowdsAbilityState>();
			state.Initialized(dataStores);
		}
		
		if (sourceState.GetPtr<FFluxPrimeCrowdsIdleState>())
		{
			const int32 currentIndex = RegisterStateRuntimeData<FFluxPrimeCrowdsIdleState>(sourceState, indexData);
		}
		
		if (sourceState.GetPtr<FFluxPrimeCrowdsDeadState>())
		{
			const int32 currentIndex = RegisterStateRuntimeData<FFluxPrimeCrowdsDeadState>(sourceState, indexData);
			
			FStateExecutor& Executor = RuntimeDataStateMachine[indexData].State[currentIndex];
			FFluxPrimeCrowdsDeadState& state = Executor.Data->GetMutable<FFluxPrimeCrowdsDeadState>();
			state.Initialized(dataStores);
		}
	}
	 
	void InitializedTransitionData(TArray<FFluxPrimeArchetypeDataStore>& dataStores, int32 indexData, 
		FInstancedStruct& sourceTransition, const FGetStateID getStateID)
	{
		if (sourceTransition.GetPtr<FFluxPrimeCrowdsStateTransition>())
		{
			const int32 currentIndex = RegisterTransitionRuntimeData<FFluxPrimeCrowdsStateTransition>(sourceTransition, indexData);
			
			FFluxPrimeCrowdsStateTransitionContext context;
			context.getStateID = getStateID;
			context.dataStores = &dataStores;
			
			FStateTransitionExecutor& Executor = RuntimeDataStateMachine[indexData].Transition[currentIndex];
			FFluxPrimeCrowdsStateTransition& transition = Executor.Data->GetMutable<FFluxPrimeCrowdsStateTransition>();
			transition.Initialized(context);
		}
	}
	
	void RegisterStateData(TArray<FFluxPrimeArchetypeDataStore>& dataStores, TArray<FFluxPrimeArchetypeEntityCatalog>& entityCatalog, 
		const int32 entityCatalogCount, const FGetStateID getStateID)
	{
		RuntimeDataStateMachine.Reserve(entityCatalogCount);
		RuntimeDataStateMachine.Init(FFluxPrimeRuntimeDataStateMapping(), entityCatalogCount);
		
		for (int indexCatalog = 0; indexCatalog < entityCatalogCount; ++indexCatalog)
		{
			TArray<FFluxPrimeStateDataEditor>& tempData = entityCatalog[indexCatalog].EntityIdentity->StateIdentity->StateData;
			RuntimeDataStateMachine[indexCatalog].State.Reserve(tempData.Num());
			RuntimeDataStateMachine[indexCatalog].Transition.Reserve(tempData.Num());

			for (auto& pair : tempData)
			{
				InitializedStateData(dataStores, indexCatalog, pair.State);
				InitializedTransitionData(dataStores, indexCatalog, pair.Transition, getStateID);
			}
		}
	}
	
public:
	void Initialized(FFluxPrimeCrowdsStateMachineSystemContext context)
	{
		check(context.getStateID);
		check(context.entityCatalog);
		check(context.dataStores);

		for (auto& dataStore : *context.dataStores)
		{
			EntityDataStateID.Add(dataStore.GetDataID(DataTagConfig.EntityDataStateTag));
			
			RegisterStateData(*context.dataStores, *context.entityCatalog, context.entityCatalog->Num(), context.getStateID);
		}
		
	}
};