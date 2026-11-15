#pragma once

#include "CoreMinimal.h"
#include "StateLogics/FluxPrimeCrowdsWalkState.h"
#include "FluxPrimeCrowdsStateTransition.h"
#include "Framework/Data/FluxPrimeIdentityData.h"
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

USTRUCT()
struct FFluxPrimeRuntimeArchetypeStateData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FFluxPrimeStateDataEditor> StateData;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsStateMachineSystem : public FFluxPrimeBaseStateMachineSystem
{
	GENERATED_BODY()
	
private:
	/** Per-archetype deep copies of the asset's state data. Executors point only into these copies. */
	UPROPERTY()
	TArray<FFluxPrimeRuntimeArchetypeStateData> RuntimeStateData;

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
	
	void RegisterStateData(TArray<FFluxPrimeArchetypeDataStore>& dataStores, const FGetStateID getStateID)
	{
		const int32 entityCatalogCount = RuntimeStateData.Num();
		RuntimeDataStateMachine.Reserve(entityCatalogCount);
		RuntimeDataStateMachine.Init(FFluxPrimeRuntimeDataStateMapping(), entityCatalogCount);
		
		for (int indexCatalog = 0; indexCatalog < entityCatalogCount; ++indexCatalog)
		{
			TArray<FFluxPrimeStateDataEditor>& tempData = RuntimeStateData[indexCatalog].StateData;
			RuntimeDataStateMachine[indexCatalog].State.Reserve(tempData.Num());
			RuntimeDataStateMachine[indexCatalog].Transition.Reserve(tempData.Num());

			for (auto& pair : tempData)
			{
				InitializedStateData(dataStores, indexCatalog, pair.State);
				InitializedTransitionData(dataStores, indexCatalog, pair.Transition, getStateID);
			}
		}
	}
	
	/** Copies each archetype's authored state and transition fragments so runtime data never touches the asset. */
	bool BuildRuntimeStateData(const TArray<FFluxPrimeArchetypeEntityCatalog>& entityCatalog)
	{
		RuntimeStateData.Reset();
		RuntimeStateData.Reserve(entityCatalog.Num());

		for (int32 indexCatalog = 0; indexCatalog < entityCatalog.Num(); ++indexCatalog)
		{
			const UFluxPrimeBaseIdentity* baseIdentity = entityCatalog[indexCatalog].EntityIdentity;
			const UFluxPrimeStateIdentity* stateIdentity = baseIdentity ? baseIdentity->StateIdentity : nullptr;
			if (!stateIdentity)
			{
				UE_LOG(LogFluxPrime, Error,
					TEXT("State machine system cannot initialize: archetype index %d has no StateIdentity."), indexCatalog);
				return false;
			}

			RuntimeStateData.Add(FFluxPrimeRuntimeArchetypeStateData());
			RuntimeStateData.Last().StateData = stateIdentity->StateData;
		}

		return true;
	}
	
public:
	void Initialized(FFluxPrimeCrowdsStateMachineSystemContext context)
	{
		check(context.getStateID);
		check(context.entityCatalog);
		check(context.dataStores);

		if (!BuildRuntimeStateData(*context.entityCatalog)) return;
		
		EntityDataStateID.Init(0, context.dataStores->Num());

		int32 indexDataId = 0;
		for (auto& dataStore : *context.dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataStateConfig, EntityDataStateID[indexDataId]);
			indexDataId++;
		}

		RegisterStateData(*context.dataStores, context.getStateID);
	}

	/** Runtime copies of the authored state data, one entry per archetype. */
	const TArray<FFluxPrimeRuntimeArchetypeStateData>& GetRuntimeStateData() const { return RuntimeStateData; }
};