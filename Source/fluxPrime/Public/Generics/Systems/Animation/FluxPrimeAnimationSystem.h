#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "Generics/Data/FluxPrimeAnimationData.h"
#include "Generics/Systems/FluxPrimeSpatialGridSystem.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "Framework/Data/FluxPrimeStateData.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Generics/Data/FluxPrimeAnimationIdentity.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperFloat.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperBool.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeAnimationSystem.generated.h"

USTRUCT(BlueprintType)
struct FAnimationSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataIdConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataStateConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataAnimStateConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataStartTimeAnimConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataPreviousFrameAnimConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataCellIdConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataTargetLocationConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRequestAnimationNotifyConfig;

	bool IsValid() const
	{
		return EntityDataIdConfig.EntityDataTag.IsValid()
		   && EntityDataStateConfig.EntityDataTag.IsValid() 
		   && EntityDataAnimStateConfig.EntityDataTag.IsValid() 
		   && EntityDataStartTimeAnimConfig.EntityDataTag.IsValid() 
		   && EntityDataPreviousFrameAnimConfig.EntityDataTag.IsValid() 
		   && EntityDataCellIdConfig.EntityDataTag.IsValid() 
		   && EntityDataTargetLocationConfig.EntityDataTag.IsValid() 
		   && EntityDataRequestAnimationNotifyConfig.EntityDataTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationSystemsContext
{
	GENERATED_BODY()
	
	FGetStateID getStateID = nullptr;
	FGetNotifyID getNotifyID = nullptr;
	/** Size of the manager's global state table. Per-archetype animation arrays are indexed by it. */
	int32 globalStateCount = 0;
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* componentsISMC = nullptr;
	/** One resolved animation asset per archetype, in catalog order. */
	const TArray<TObjectPtr<UFluxPrimeAnimationData>>* resolvedAnimationData = nullptr;
	
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	FGetStateID GetStateID;
	FGetNotifyID GetNotifyID;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* ComponentsISMC = nullptr;
	
	UPROPERTY()
	TArray<FFluxPrimeRuntimeAnimationMapping> RuntimeAnimationMapping;
	
	UPROPERTY()
	TArray<FFluxPrimeRuntimeAnimationNotify> RuntimeAnimationNotify;
	
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
	UPROPERTY(EditAnywhere)
	FFluxPrimeSpatialGridSystem SpatialGridSystem = FFluxPrimeSpatialGridSystem();
	
	UPROPERTY(EditAnywhere)
	FAnimationSystemDataTagConfig DataTagConfig;

	UPROPERTY(EditAnywhere, Category = "Animation", meta = (ClampMin = "0.001"))
	float SourceFrameRate = 30.0f;
	
	TArray<int16> EntityDataIdID, EntityDataStateID, EntityDataAnimStateID, EntityDataStartTimeAnimID, EntityDataPreviousFrameAnimID, EntityDataCellIdID, EntityDataTargetLocationID, EntityDataRequestAnimationNotifyID;
	
public:
	/** Copies presentation only; datastore relocation already preserves playback and notify phase. */
	TFunction<void(int32, int32, int32)> CreatePresentationRelocator() const
	{
		TArray<TWeakObjectPtr<UInstancedStaticMeshComponent>> Components;
		if (ComponentsISMC)
		{
			for (const auto& Component : *ComponentsISMC) Components.Add(Component.Get());
		}
		// No system/module address survives FInstancedStruct copies or teardown.
		return [Components = MoveTemp(Components)](int32 Archetype, int32 Source, int32 Destination)
		{
			if (Source == Destination || !Components.IsValidIndex(Archetype)) return;
			UInstancedStaticMeshComponent* Component = Components[Archetype].Get();
			if (!Component || Component->NumCustomDataFloats < 6 || Source < 0 || Destination < 0 ||
				Source >= Component->GetInstanceCount() || Destination >= Component->GetInstanceCount()) return;
			for (int32 Field = 0; Field < 6; ++Field)
			{
				Component->SetCustomDataValue(Destination, Field,
					Component->PerInstanceSMCustomData[Source * Component->NumCustomDataFloats + Field], false);
			}
			Component->MarkRenderStateDirty();
		};
	}

	/** Builds per-archetype runtime mappings indexed by the manager's global state ids. */
	bool InitializeAnimationMapping(const TArray<TObjectPtr<UFluxPrimeAnimationData>>& resolvedAnimationData,
		int32 globalStateCount)
	{
		const int32 countCatalog = resolvedAnimationData.Num();

		if (globalStateCount <= 0)
		{
			UE_LOG(LogFluxPrime, Error,
				TEXT("Animation system cannot initialize: the global state table is empty."));
			return false;
		}

		if (globalStateCount > FluxPrimeMaxGlobalStateCount)
		{
			UE_LOG(LogFluxPrime, Error,
				TEXT("Animation system cannot initialize: global state count %d exceeds the int8 runtime carrier ")
				TEXT("limit of %d."),
				globalStateCount, FluxPrimeMaxGlobalStateCount);
			return false;
		}

		RuntimeAnimationMapping.Init(FFluxPrimeRuntimeAnimationMapping(), countCatalog);
		RuntimeAnimationNotify.Init(FFluxPrimeRuntimeAnimationNotify(), countCatalog);
		
		for (int i = 0; i < countCatalog; ++i)
		{
			UFluxPrimeAnimationData* dataAnim = resolvedAnimationData[i];
			if (!dataAnim)
			{
				UE_LOG(LogFluxPrime, Error,
					TEXT("Animation system cannot initialize archetype %d: no animation asset is resolved."), i);
				return false;
			}

			FFluxPrimeRuntimeAnimationMapping& mapping = RuntimeAnimationMapping[i];
			FFluxPrimeRuntimeAnimationNotify& notify = RuntimeAnimationNotify[i];

			mapping.AnimationStart.Init(0, globalStateCount);
			mapping.AnimationEnd.Init(0, globalStateCount);
			mapping.AnimationForceNotify.Init(false, globalStateCount);
			mapping.AnimationStateValid.Init(false, globalStateCount);
			notify.RuntimeAnimationNotifyData.Init(FFluxPrimeRuntimeAnimationNotifyData(), globalStateCount);
			
			for (auto& pair : dataAnim->DataAnimations)
			{
				const int32 index = GetStateID(pair.StateTag);
				if (index == INDEX_NONE || index >= globalStateCount)
				{
					UE_LOG(LogFluxPrime, Error,
						TEXT("Animation system cannot initialize archetype %d: state tag '%s' is not registered ")
						TEXT("in the global state table (count %d)."),
						i, *pair.StateTag.ToString(), globalStateCount);
					return false;
				}

				if (pair.AnimationStartFrame < 0 || pair.AnimationEndFrame < pair.AnimationStartFrame)
				{
					UE_LOG(LogFluxPrime, Error,
						TEXT("Animation system cannot initialize archetype %d: state '%s' has invalid frames %d-%d."),
						i, *pair.StateTag.ToString(), pair.AnimationStartFrame, pair.AnimationEndFrame);
					return false;
				}
				
				for (int32 l = 0; l < FluxConfig::AnimationArrayCount; ++l)
				{
					if (!pair.AnimationNotifies[l].AnimationNotifyTag.IsValid()) continue;

					const int32 notifyID = GetNotifyID(pair.AnimationNotifies[l].AnimationNotifyTag);
					if (notifyID == INDEX_NONE)
					{
						UE_LOG(LogFluxPrime, Error,
							TEXT("Animation system cannot initialize archetype %d: notify tag '%s' is not registered."),
							i, *pair.AnimationNotifies[l].AnimationNotifyTag.ToString());
						return false;
					}

					const int32 notifyCount = notify.RuntimeAnimationNotifyData[index].AnimationNotifyCount;
					if (notifyCount < 0 || notifyCount >= FluxConfig::AnimationArrayCount)
					{
						UE_LOG(LogFluxPrime, Error,
							TEXT("Animation system cannot initialize archetype %d: state '%s' has more than %d notifies."),
							i, *pair.StateTag.ToString(), FluxConfig::AnimationArrayCount);
						return false;
					}

					notify.RuntimeAnimationNotifyData[index].AnimationNotify[notifyCount].AnimationNotifyID = notifyID;
					notify.RuntimeAnimationNotifyData[index].AnimationNotify[notifyCount].AnimationNotifyFrame = pair.AnimationNotifies[l].AnimationNotifyFrame;
					notify.RuntimeAnimationNotifyData[index].AnimationNotifyCount = static_cast<int8>(notifyCount + 1);
				}
				
				mapping.AnimationStart[index] = pair.AnimationStartFrame;
				mapping.AnimationEnd[index] = pair.AnimationEndFrame;
				mapping.AnimationForceNotify[index] = pair.AnimationForceNotify;
				mapping.AnimationStateValid[index] = true;
			}
		}

		return countCatalog > 0;
	}

	const TArray<FFluxPrimeRuntimeAnimationMapping>& GetRuntimeAnimationMapping() const
	{
		return RuntimeAnimationMapping;
	}
	
private:
	bool IsValidAnimationState(int32 type, int32 state) const
	{
		return RuntimeAnimationMapping.IsValidIndex(type) &&
			RuntimeAnimationMapping[type].AnimationStart.IsValidIndex(state) &&
			RuntimeAnimationMapping[type].AnimationStateValid.IsValidIndex(state) &&
			RuntimeAnimationMapping[type].AnimationStateValid[state] &&
			RuntimeAnimationNotify.IsValidIndex(type) &&
			RuntimeAnimationNotify[type].RuntimeAnimationNotifyData.IsValidIndex(state);
	}

	bool IsNotifyTriggered(float PrevFrame, float CurrentFrame, float NotifyFrame)
	{
		if (PrevFrame < 0.0f) return false;
		
		if (CurrentFrame >= PrevFrame)
		{
			return NotifyFrame > PrevFrame && NotifyFrame <= CurrentFrame;
		}

		return NotifyFrame > PrevFrame || NotifyFrame <= CurrentFrame;
	}
	
	void PlayAnimation(int8& animationState, float& startTimeAnimation, 
		int8& crowdType, float& previousFrameAnimation, int32& requestAnimationNotify, float realTime)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_PlayAnimation);
		
		if (!IsValidAnimationState(crowdType, animationState)) return;

		float startFrame = RuntimeAnimationMapping[crowdType].AnimationStart[animationState];
		float endFrame = RuntimeAnimationMapping[crowdType].AnimationEnd[animationState];
		
		float localAnimTime = realTime - startTimeAnimation;
		float realTimeFrames = localAnimTime * SourceFrameRate;
		float reminder = FMath::Fmod(realTimeFrames, (endFrame - startFrame) + 1.0f);
		float current = startFrame + reminder;
		
		if (RuntimeAnimationNotify[crowdType].RuntimeAnimationNotifyData[animationState].AnimationNotifyCount > 0) PlayAnimationNotify(RuntimeAnimationNotify[crowdType], requestAnimationNotify, animationState,current, previousFrameAnimation);
		previousFrameAnimation = current;
	}
	
	void PlayAnimationNotify(FFluxPrimeRuntimeAnimationNotify& notify, int32& requestAnimationNotify, uint8 indexAnimation,float currentFrame, float previousFrame)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_PlayNotifyAnimation);
		
		for (int i = 0; i < notify.RuntimeAnimationNotifyData[indexAnimation].AnimationNotifyCount; ++i)
		{	
			if (!IsNotifyTriggered(previousFrame, currentFrame, notify.RuntimeAnimationNotifyData[indexAnimation].AnimationNotify[i].AnimationNotifyFrame)) continue;

			requestAnimationNotify = notify.RuntimeAnimationNotifyData[indexAnimation].AnimationNotify[i].AnimationNotifyID;
		}
	}
	
	void UpdateAnimation(int16& instanceIndexEntity, int8& type, int8& state, 
		int8& animationState, float& startTimeAnimation, float& previousFrameAnimation, float realTime)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_UpdateAnimation);
		
		if (!IsValidAnimationState(type, animationState) || !IsValidAnimationState(type, state)) return;

		if (animationState != state)
		{
			// previous
			uint8 indexAnimation = animationState;
			uint16 previousStartFrame = RuntimeAnimationMapping[type].AnimationStart[indexAnimation];
			uint16 previousEndFrame = RuntimeAnimationMapping[type].AnimationEnd[indexAnimation];
			const float previousTime = startTimeAnimation;
			
			// change current state
			animationState = state;
			
			// current
			indexAnimation = animationState;
			uint16 currentStartFrame = RuntimeAnimationMapping[type].AnimationStart[indexAnimation];
			uint16 currentEndFrame = RuntimeAnimationMapping[type].AnimationEnd[indexAnimation];
			
			SwitchAnimation(instanceIndexEntity, type, currentStartFrame, currentEndFrame, previousStartFrame, previousEndFrame, previousTime, realTime);
			
			// update start time
			startTimeAnimation = realTime;
			previousFrameAnimation = -1.0f;
		}
	}
	
	void SwitchAnimation(uint16 instanceIndexEntity, int8 type, uint16 currentStartFrame, uint16 currentEndFrame, uint16 previousStartFrame, uint16 previousEndFrame, float previousStartTime, float realTime)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_Switch);
		
		check(ComponentsISMC->IsValidIndex(type));
	
		// previous
		(*ComponentsISMC)[type]->SetCustomDataValue(
			instanceIndexEntity,
			0,
			previousStartTime,
			false
			);
	
		(*ComponentsISMC)[type]->SetCustomDataValue(
			instanceIndexEntity,
			1,
			previousStartFrame,
			false
			);
	
		(*ComponentsISMC)[type]->SetCustomDataValue(
			instanceIndexEntity,
			2,
			previousEndFrame,
			false
			);
		
		// current
		(*ComponentsISMC)[type]->SetCustomDataValue(
			instanceIndexEntity,
			3,
			realTime,
			false
			);
	
		(*ComponentsISMC)[type]->SetCustomDataValue(
			instanceIndexEntity,
			4,
			currentStartFrame,
			false
			);
	
		(*ComponentsISMC)[type]->SetCustomDataValue(
			instanceIndexEntity,
			5,
			currentEndFrame,
			false
			);
	}
	
public:
	void Initialized(FFluxPrimeAnimationSystemsContext context)
	{
		check(context.world);
		check(context.componentsISMC);
		check(context.resolvedAnimationData);
		check(context.getStateID);
		check(context.getNotifyID);
		check(context.dataStores);

		IsAllDataValid = true;

		int32 countData = context.dataStores->Num();
		EntityDataIdID.Reset();
		EntityDataStateID.Reset();
		EntityDataAnimStateID.Reset();
		EntityDataStartTimeAnimID.Reset();
		EntityDataPreviousFrameAnimID.Reset();
		EntityDataCellIdID.Reset();
		EntityDataTargetLocationID.Reset();
		EntityDataRequestAnimationNotifyID.Reset();
		EntityDataIdID.Init(0, countData);
		EntityDataStateID.Init(0, countData);
		EntityDataAnimStateID.Init(0, countData);
		EntityDataStartTimeAnimID.Init(0, countData);
		EntityDataPreviousFrameAnimID.Init(0, countData);
		EntityDataCellIdID.Init(0, countData);
		EntityDataTargetLocationID.Init(0, countData);
		EntityDataRequestAnimationNotifyID.Init(0, countData);
		
		World = context.world;
		ComponentsISMC = context.componentsISMC;
		GetStateID = context.getStateID;
		GetNotifyID = context.getNotifyID;
		
		if (!InitializeAnimationMapping(*context.resolvedAnimationData, context.globalStateCount))
		{
			IsAllDataValid = false;
			return;
		}
		
		SpatialGridSystem.Initialized(*context.dataStores);
		
		int32 indexDataId = 0;
		for (auto& dataStore : *context.dataStores)
		{
			TryInitializedData(dataStore, DataTagConfig.EntityDataIdConfig, EntityDataIdID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataStateConfig, EntityDataStateID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataAnimStateConfig, EntityDataAnimStateID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataStartTimeAnimConfig, EntityDataStartTimeAnimID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataPreviousFrameAnimConfig, EntityDataPreviousFrameAnimID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataCellIdConfig, EntityDataCellIdID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataTargetLocationConfig, EntityDataTargetLocationID[indexDataId]);
			TryInitializedData(dataStore, DataTagConfig.EntityDataRequestAnimationNotifyConfig, EntityDataRequestAnimationNotifyID[indexDataId]);
			indexDataId++;
		}
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_Update);
		
		if (!IsAllDataValid)
		{
			UE_LOG(LogFluxPrime, Verbose, TEXT("ANIMATION SYSTEM :: data invalid; system skipped"));
			return;
		}
		
		SpatialGridSystem.UpdateSpatialGridSystem(dataStores);
		
		float realTime = World->GetTimeSeconds();

		int8 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			FFluxPrimeWrapperInt16* entityDataId = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataIdID[indexDataID]));
			FFluxPrimeWrapperInt8* entityDataState = reinterpret_cast<FFluxPrimeWrapperInt8*>(dataStore.GetRawBufferData(EntityDataStateID[indexDataID]));
			FFluxPrimeWrapperInt8* entityDataAnimState = reinterpret_cast<FFluxPrimeWrapperInt8*>(dataStore.GetRawBufferData(EntityDataAnimStateID[indexDataID]));
			FFluxPrimeWrapperFloat* entityDataStartTimeAnim = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataStartTimeAnimID[indexDataID]));
			FFluxPrimeWrapperFloat* entityDataPreviousFrameAnim = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataPreviousFrameAnimID[indexDataID]));
			FFluxPrimeWrapperInt32* entityDataCellId = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataCellIdID[indexDataID]));
			FVector* entityDataTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]));
			FFluxPrimeWrapperInt32* entityDataRequestAnimationNotify = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataRequestAnimationNotifyID[indexDataID]));
			
			check(entityDataId);
			check(entityDataState);
			check(entityDataAnimState);
			check(entityDataStartTimeAnim);
			check(entityDataPreviousFrameAnim);
			check(entityDataCellId);
			check(entityDataTargetLocation);
			check(entityDataRequestAnimationNotify);

			for (int16 i = 0; i < dataStore.GetActiveEntityCount(); ++i)
			{
				const int8 state = entityDataState[i].ValueInt8;
				const int8 animationState = entityDataAnimState[i].ValueInt8;
				if (!IsValidAnimationState(indexDataID, state) || !IsValidAnimationState(indexDataID, animationState)) continue;

				UpdateAnimation(i, indexDataID, entityDataState[i].ValueInt8,  entityDataAnimState[i].ValueInt8, 
					entityDataStartTimeAnim[i].ValueFloat, entityDataPreviousFrameAnim[i].ValueFloat, realTime);
				
				int32 targetCellId = SpatialGridSystem.GetSpatialGridSystemsCellID(entityDataTargetLocation[i]);
				
				if (entityDataCellId[i].ValueInt32 != targetCellId && !RuntimeAnimationMapping[indexDataID].AnimationForceNotify[entityDataState[i].ValueInt8]) continue;
			
				PlayAnimation(entityDataAnimState[i].ValueInt8, entityDataStartTimeAnim[i].ValueFloat, indexDataID,
					entityDataPreviousFrameAnim[i].ValueFloat, entityDataRequestAnimationNotify[i].ValueInt32, realTime);
			}
			
			indexDataID++;
		}
	}
};