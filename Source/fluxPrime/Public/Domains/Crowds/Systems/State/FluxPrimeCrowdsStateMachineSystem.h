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
	/** Size of the manager's global state table. Executor arrays are indexed by it. */
	int32 globalStateCount = 0;
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

	void InitializedStateData(TArray<FFluxPrimeArchetypeDataStore>& dataStores, int32 indexData, int32 targetState,
		FInstancedStruct& sourceState)
	{
		if (sourceState.GetPtr<FFluxPrimeCrowdsWalkState>())
		{
			const int32 currentIndex = RegisterStateRuntimeData<FFluxPrimeCrowdsWalkState>(sourceState, indexData, targetState);
			if (currentIndex == INDEX_NONE) return;
		}
		
		if (sourceState.GetPtr<FFluxPrimeCrowdsAbilityState>())
		{
			const int32 currentIndex = RegisterStateRuntimeData<FFluxPrimeCrowdsAbilityState>(sourceState, indexData, targetState);
			if (currentIndex == INDEX_NONE) return;
			
			FStateExecutor& Executor = RuntimeDataStateMachine[indexData].State[currentIndex];
			FFluxPrimeCrowdsAbilityState& state = Executor.Data->GetMutable<FFluxPrimeCrowdsAbilityState>();
			state.Initialized(dataStores);
		}
		
		if (sourceState.GetPtr<FFluxPrimeCrowdsIdleState>())
		{
			const int32 currentIndex = RegisterStateRuntimeData<FFluxPrimeCrowdsIdleState>(sourceState, indexData, targetState);
			if (currentIndex == INDEX_NONE) return;
		}
		
		if (sourceState.GetPtr<FFluxPrimeCrowdsDeadState>())
		{
			const int32 currentIndex = RegisterStateRuntimeData<FFluxPrimeCrowdsDeadState>(sourceState, indexData, targetState);
			if (currentIndex == INDEX_NONE) return;
			
			FStateExecutor& Executor = RuntimeDataStateMachine[indexData].State[currentIndex];
			FFluxPrimeCrowdsDeadState& state = Executor.Data->GetMutable<FFluxPrimeCrowdsDeadState>();
			state.Initialized(dataStores);
		}
	}
	 
	void InitializedTransitionData(TArray<FFluxPrimeArchetypeDataStore>& dataStores, int32 indexData, int32 targetState,
		FInstancedStruct& sourceTransition, const FGetStateID getStateID)
	{
		if (sourceTransition.GetPtr<FFluxPrimeCrowdsStateTransition>())
		{
			const int32 currentIndex = RegisterTransitionRuntimeData<FFluxPrimeCrowdsStateTransition>(sourceTransition, indexData, targetState);
			if (currentIndex == INDEX_NONE) return;
			
			FFluxPrimeCrowdsStateTransitionContext context;
			context.getStateID = getStateID;
			context.dataStores = &dataStores;
			
			FStateTransitionExecutor& Executor = RuntimeDataStateMachine[indexData].Transition[currentIndex];
			FFluxPrimeCrowdsStateTransition& transition = Executor.Data->GetMutable<FFluxPrimeCrowdsStateTransition>();
			transition.Initialized(context);
		}
	}
	
	bool RegisterStateData(TArray<FFluxPrimeArchetypeDataStore>& dataStores, const FGetStateID getStateID,
		int32 globalStateCount)
	{
		if (globalStateCount <= 0)
		{
			UE_LOG(LogFluxPrime, Error,
				TEXT("State machine system cannot register states: the global state table is empty."));
			return false;
		}

		if (globalStateCount > FluxPrimeMaxGlobalStateCount)
		{
			UE_LOG(LogFluxPrime, Error,
				TEXT("State machine system cannot register states: global state count %d exceeds the int8 runtime ")
				TEXT("carrier limit of %d."),
				globalStateCount, FluxPrimeMaxGlobalStateCount);
			return false;
		}

		const int32 entityCatalogCount = RuntimeStateData.Num();
		RuntimeDataStateMachine.Init(FFluxPrimeRuntimeDataStateMapping(), entityCatalogCount);
		
		for (int indexCatalog = 0; indexCatalog < entityCatalogCount; ++indexCatalog)
		{
			TArray<FFluxPrimeStateDataEditor>& tempData = RuntimeStateData[indexCatalog].StateData;
			RuntimeDataStateMachine[indexCatalog].State.SetNum(globalStateCount);
			RuntimeDataStateMachine[indexCatalog].Transition.SetNum(globalStateCount);

			TArray<bool> stateAssigned;
			stateAssigned.Init(false, globalStateCount);

			for (auto& pair : tempData)
			{
				const int32 targetState = getStateID(pair.IdentityState);
				if (targetState == INDEX_NONE || targetState >= globalStateCount)
				{
					UE_LOG(LogFluxPrime, Error,
						TEXT("State machine system cannot initialize archetype %d: state tag '%s' is not registered ")
						TEXT("in the global state table (count %d)."),
						indexCatalog, *pair.IdentityState.ToString(), globalStateCount);
					RuntimeDataStateMachine.Reset();
					return false;
				}

				if (stateAssigned[targetState])
				{
					UE_LOG(LogFluxPrime, Error,
						TEXT("State machine system cannot initialize archetype %d: state tag '%s' is authored more ")
						TEXT("than once (global state id %d)."),
						indexCatalog, *pair.IdentityState.ToString(), targetState);
					RuntimeDataStateMachine.Reset();
					return false;
				}

				stateAssigned[targetState] = true;

				InitializedStateData(dataStores, indexCatalog, targetState, pair.State);
				InitializedTransitionData(dataStores, indexCatalog, targetState, pair.Transition, getStateID);
			}
		}

		return true;
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

		IsAllDataValid = true;
		EntityDataStateID.Reset();
		// Executors point into RuntimeStateData; drop both together so a failed rebuild cannot leave stale pointers.
		RuntimeDataStateMachine.Reset();

		if (!BuildRuntimeStateData(*context.entityCatalog))
		{
			IsAllDataValid = false;
			return;
		}
		
		EntityDataStateID.Init(0, context.dataStores->Num());

		int32 indexDataId = 0;
		for (auto& dataStore : *context.dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataStateConfig, EntityDataStateID[indexDataId]);
			indexDataId++;
		}

		if (!IsAllDataValid) return;

		if (!RegisterStateData(*context.dataStores, context.getStateID, context.globalStateCount))
		{
			IsAllDataValid = false;
		}
	}

	/** Runtime copies of the authored state data, one entry per archetype. */
	const TArray<FFluxPrimeRuntimeArchetypeStateData>& GetRuntimeStateData() const { return RuntimeStateData; }

	/** Runtime executor tables, indexed by archetype and then by global state id. */
	const TArray<FFluxPrimeRuntimeDataStateMapping>& GetRuntimeDataStateMachine() const { return RuntimeDataStateMachine; }
};