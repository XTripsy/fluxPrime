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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FFluxPrimeDataConfig EntityDataRequestSyncAnimationConfig;

	bool IsValid() const
	{
		return EntityDataIdConfig.EntityDataTag.IsValid()
		   && EntityDataStateConfig.EntityDataTag.IsValid() 
		   && EntityDataAnimStateConfig.EntityDataTag.IsValid() 
		   && EntityDataStartTimeAnimConfig.EntityDataTag.IsValid() 
		   && EntityDataPreviousFrameAnimConfig.EntityDataTag.IsValid() 
		   && EntityDataCellIdConfig.EntityDataTag.IsValid() 
		   && EntityDataTargetLocationConfig.EntityDataTag.IsValid() 
		   && EntityDataRequestAnimationNotifyConfig.EntityDataTag.IsValid()
		   && EntityDataRequestSyncAnimationConfig.EntityDataTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationSystemsContext
{
	GENERATED_BODY()
	
	FGetStateID getStateID = nullptr;
	FGetNotifyID getNotifyID = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* componentsISMC = nullptr;
	TArray<FName>* nameCatalog = nullptr;
	uint16 countCatalog;
	TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>>* animationSoftRef = nullptr;
	
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
	
	TArray<int16> EntityDataIdID, EntityDataStateID, EntityDataAnimStateID, EntityDataStartTimeAnimID, EntityDataPreviousFrameAnimID, EntityDataCellIdID, EntityDataTargetLocationID, EntityDataRequestAnimationNotifyID, EntityDataRequestSyncAnimationID;
	
private:
	void InitializeAnimationMapping(const uint16& countCatalog, TArray<FName>& nameCatalog, TMap<FName, TSoftObjectPtr<UFluxPrimeAnimationData>>& animationSoftRef)
	{
		RuntimeAnimationMapping.Init(FFluxPrimeRuntimeAnimationMapping(), countCatalog);
		RuntimeAnimationNotify.Init(FFluxPrimeRuntimeAnimationNotify(), countCatalog);
		
		int32 notifyIndex = -1;
		
		for (int i = 0; i < countCatalog; ++i)
		{
			UFluxPrimeAnimationData& dataAnim = *animationSoftRef[nameCatalog[i]];
			
			int32 countAnimationData = dataAnim.DataAnimations.Num();
			FFluxPrimeRuntimeAnimationMapping animationRuntimeData = FFluxPrimeRuntimeAnimationMapping();
			FFluxPrimeRuntimeAnimationNotify animationRuntimeNotify = FFluxPrimeRuntimeAnimationNotify();
			
			animationRuntimeData.AnimationStart.Init(0, countAnimationData);
			animationRuntimeData.AnimationEnd.Init(0, countAnimationData);
			animationRuntimeData.AnimationForceNotify.Init(false, countAnimationData);
			animationRuntimeNotify.RuntimeAnimationNotifyData.Init(FFluxPrimeRuntimeAnimationNotifyData(), countAnimationData);
			
			RuntimeAnimationMapping[i].AnimationStart.Init(0, countAnimationData);
			RuntimeAnimationMapping[i].AnimationEnd.Init(0, countAnimationData);
			RuntimeAnimationMapping[i].AnimationForceNotify.Init(false, countAnimationData);
			
			RuntimeAnimationNotify[i].RuntimeAnimationNotifyData.Init(FFluxPrimeRuntimeAnimationNotifyData(), countAnimationData);
			
			for (auto& pair : dataAnim.DataAnimations)
			{
				int32 index = GetStateID(pair.StateTag);
				
				for (int32 l = 0; l < FluxConfig::AnimationArrayCount; ++l)
				{
					if (!pair.AnimationNotifies[l].AnimationNotifyTag.IsValid()) continue;
					notifyIndex = animationRuntimeNotify.RuntimeAnimationNotifyData[index].AnimationNotifyCount;
					animationRuntimeNotify.RuntimeAnimationNotifyData[index].AnimationNotify[notifyIndex].AnimationNotifyID = GetNotifyID(pair.AnimationNotifies[l].AnimationNotifyTag);
					animationRuntimeNotify.RuntimeAnimationNotifyData[index].AnimationNotify[notifyIndex].AnimationNotifyFrame = pair.AnimationNotifies[l].AnimationNotifyFrame;
					animationRuntimeNotify.RuntimeAnimationNotifyData[index].AnimationNotifyCount++;
				}
				
				animationRuntimeData.AnimationStart[index] = pair.AnimationStartFrame;
				animationRuntimeData.AnimationEnd[index] = pair.AnimationEndFrame;
				animationRuntimeData.AnimationForceNotify[index] = pair.AnimationForceNotify;
			}

			RuntimeAnimationMapping[i] = animationRuntimeData;
			RuntimeAnimationNotify[i] = animationRuntimeNotify;
		}
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
		
		float startFrame = RuntimeAnimationMapping[crowdType].AnimationStart[animationState];
		float endFrame = RuntimeAnimationMapping[crowdType].AnimationEnd[animationState];
		
		float localAnimTime = realTime - startTimeAnimation;
		float realTimeFrames = localAnimTime * 30.0f;
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
		
		if (animationState != state)
		{
			// previous
			uint8 indexAnimation = animationState;
			uint16 previousStartFrame = RuntimeAnimationMapping[type].AnimationStart[indexAnimation];
			uint16 previousEndFrame = RuntimeAnimationMapping[type].AnimationEnd[indexAnimation];
			uint16 previousTime = startTimeAnimation;
			
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
	
	void SyncAnimation(const int16 instanceIndexEntity, const int8 type, const int8 animationState, 
		float& startTimeAnimation, float& previousFrameAnimation, float realTime)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_UpdateAnimation);
		
		// previous
		uint16 previousStartFrame = RuntimeAnimationMapping[type].AnimationStart[animationState];
		uint16 previousEndFrame = RuntimeAnimationMapping[type].AnimationEnd[animationState];
		uint16 previousTime = startTimeAnimation;
		
		// current
		uint16 currentStartFrame = RuntimeAnimationMapping[type].AnimationStart[animationState];
		uint16 currentEndFrame = RuntimeAnimationMapping[type].AnimationEnd[animationState];
			
		SwitchAnimation(instanceIndexEntity, type, currentStartFrame, currentEndFrame, previousStartFrame, previousEndFrame, previousTime, realTime);
		
		// update start time
		startTimeAnimation = realTime;
		previousFrameAnimation = -1.0f;
	}
	
	void SwitchAnimation(uint16 instanceIndexEntity, int8 type, uint16 currentStartFrame, uint16 currentEndFrame, uint16 previousStartFrame, uint16 previousEndFrame,uint16 previousStartTime, float realTime)
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
		check(context.nameCatalog);
		check(context.animationSoftRef);
		check(context.getStateID);
		check(context.getNotifyID);
		check(context.dataStores);
		
		int32 countData = context.dataStores->Num();
		EntityDataIdID.Init(0, countData);
		EntityDataStateID.Init(0, countData);
		EntityDataAnimStateID.Init(0, countData);
		EntityDataStartTimeAnimID.Init(0, countData);
		EntityDataPreviousFrameAnimID.Init(0, countData);
		EntityDataCellIdID.Init(0, countData);
		EntityDataTargetLocationID.Init(0, countData);
		EntityDataRequestAnimationNotifyID.Init(0, countData);
		EntityDataRequestSyncAnimationID.Init(0, countData);
		
		World = context.world;
		ComponentsISMC = context.componentsISMC;
		GetStateID = context.getStateID;
		GetNotifyID = context.getNotifyID;
		
		InitializeAnimationMapping(context.countCatalog, *context.nameCatalog, *context.animationSoftRef);
		
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
			TryInitializedData(dataStore, DataTagConfig.EntityDataRequestSyncAnimationConfig, EntityDataRequestSyncAnimationID[indexDataId]);
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
		
		float realTime = World->GetRealTimeSeconds();

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
			FFluxPrimeWrapperBool* entityDataRequestSyncNotify = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataRequestSyncAnimationID[indexDataID]));
			
			check(entityDataId);
			check(entityDataState);
			check(entityDataAnimState);
			check(entityDataStartTimeAnim);
			check(entityDataPreviousFrameAnim);
			check(entityDataCellId);
			check(entityDataTargetLocation);
			check(entityDataRequestAnimationNotify);
			check(entityDataRequestSyncNotify);

			for (int16 i = 0; i < dataStore.GetActiveEntityCount(); ++i)
			{
				if (entityDataRequestSyncNotify[i].ValueBool)
				{
					entityDataRequestSyncNotify[i].ValueBool = false;
					SyncAnimation(i, indexDataID,  
					entityDataAnimState[i].ValueInt8, entityDataStartTimeAnim[i].ValueFloat, entityDataPreviousFrameAnim[i].ValueFloat, realTime);
				}
				
				UpdateAnimation(i, indexDataID, entityDataState[i].ValueInt8, 
					entityDataAnimState[i].ValueInt8, entityDataStartTimeAnim[i].ValueFloat, entityDataPreviousFrameAnim[i].ValueFloat, realTime);
				
				int32 targetCellId = SpatialGridSystem.GetSpatialGridSystemsCellID(entityDataTargetLocation[i]);
				
				if (entityDataCellId[i].ValueInt32 != targetCellId && !RuntimeAnimationMapping[indexDataID].AnimationForceNotify[entityDataState[i].ValueInt8]) continue;
			
				PlayAnimation(entityDataAnimState[i].ValueInt8, entityDataStartTimeAnim[i].ValueFloat, indexDataID,
					entityDataPreviousFrameAnim[i].ValueFloat, entityDataRequestAnimationNotify[i].ValueInt32, realTime);
			}
			
			indexDataID++;
		}
	}
};