#pragma once

#include "CoreMinimal.h"
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
#if IF_WITH_EDITOR
#include "ProfilingDebugging/CpuProfilerTrace.h"
#endif
#include "FluxPrimeAnimationSystem.generated.h"

USTRUCT(BlueprintType)
struct FAnimationSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataIdTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataAnimStateTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataStartTimeAnimTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataPreviousFrameAnimTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataCellIdTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataTargetLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRequestAnimationNotifyTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataRequestSyncAnimationTag;

	bool IsValid() const
	{
		return EntityDataIdTag.IsValid()
		   && EntityDataStateTag.IsValid() 
		   && EntityDataAnimStateTag.IsValid() 
		   && EntityDataStartTimeAnimTag.IsValid() 
		   && EntityDataPreviousFrameAnimTag.IsValid() 
		   && EntityDataCellIdTag.IsValid() 
		   && EntityDataTargetLocationTag.IsValid() 
		   && EntityDataRequestAnimationNotifyTag.IsValid()
		   && EntityDataRequestSyncAnimationTag.IsValid();
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
	FFluxPrimeSpatialGridSystem SpatialGridSystems;
	
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
#if IF_WITH_EDITOR
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_PlayAnimation);
#endif
		
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
#if IF_WITH_EDITOR
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_PlayNotifyAnimation);
#endif
		
		for (int i = 0; i < notify.RuntimeAnimationNotifyData[indexAnimation].AnimationNotifyCount; ++i)
		{	
			if (!IsNotifyTriggered(previousFrame, currentFrame, notify.RuntimeAnimationNotifyData[indexAnimation].AnimationNotify[i].AnimationNotifyFrame)) continue;

			requestAnimationNotify = notify.RuntimeAnimationNotifyData[indexAnimation].AnimationNotify[i].AnimationNotifyID;
		}
	}
	
	void UpdateAnimation(int16& instanceIndexEntity, int8& type, int8& state, 
		int8& animationState, float& startTimeAnimation, float& previousFrameAnimation, float realTime)
	{
#if IF_WITH_EDITOR
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_UpdateAnimation);
#endif
		
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
	
	void SyncAnimation(int16& instanceIndexEntity, int8& type, int8& animationState, 
		float& startTimeAnimation, float& previousFrameAnimation, float realTime)
	{
#if IF_WITH_EDITOR
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_UpdateAnimation);
#endif
		
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
#if IF_WITH_EDITOR
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_Switch);
#endif
		
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
		
		World = context.world;
		ComponentsISMC = context.componentsISMC;
		GetStateID = context.getStateID;
		GetNotifyID = context.getNotifyID;
		
		InitializeAnimationMapping(context.countCatalog, *context.nameCatalog, *context.animationSoftRef);
		
		SpatialGridSystems.Initialized(*context.dataStores);
		
		for (auto& dataStore : *context.dataStores)
		{
			EntityDataIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataIdTag));
			EntityDataStateID.Add(dataStore.GetDataID(DataTagConfig.EntityDataStateTag));
			EntityDataAnimStateID.Add(dataStore.GetDataID(DataTagConfig.EntityDataAnimStateTag));
			EntityDataStartTimeAnimID.Add(dataStore.GetDataID(DataTagConfig.EntityDataStartTimeAnimTag));
			EntityDataPreviousFrameAnimID.Add(dataStore.GetDataID(DataTagConfig.EntityDataPreviousFrameAnimTag));
			EntityDataCellIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataCellIdTag));
			EntityDataTargetLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataTargetLocationTag));
			EntityDataRequestAnimationNotifyID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestAnimationNotifyTag));
			EntityDataRequestSyncAnimationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataRequestSyncAnimationTag));
		}
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
#if IF_WITH_EDITOR
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Systems_Update);
#endif

		int8 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			SpatialGridSystems.UpdateSpatialGridSystem(dataStore);
			
			FFluxPrimeWrapperInt16* entityDataId = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataIdID[indexDataID]));
			FFluxPrimeWrapperInt8* entityDataState = reinterpret_cast<FFluxPrimeWrapperInt8*>(dataStore.GetRawBufferData(EntityDataStateID[indexDataID]));
			FFluxPrimeWrapperInt8* entityDataAnimState = reinterpret_cast<FFluxPrimeWrapperInt8*>(dataStore.GetRawBufferData(EntityDataAnimStateID[indexDataID]));
			FFluxPrimeWrapperFloat* entityDataStartTimeAnim = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataStartTimeAnimID[indexDataID]));
			FFluxPrimeWrapperFloat* entityDataPreviousFrameAnim = reinterpret_cast<FFluxPrimeWrapperFloat*>(dataStore.GetRawBufferData(EntityDataPreviousFrameAnimID[indexDataID]));
			FFluxPrimeWrapperInt32* entityDataCellId = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataCellIdID[indexDataID]));
			FVector* entityDataTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]));
			FFluxPrimeWrapperInt32* entityDataRequestAnimationNotify = reinterpret_cast<FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(EntityDataRequestAnimationNotifyID[indexDataID]));
			FFluxPrimeWrapperBool* entityDataRequestSyncNotify = reinterpret_cast<FFluxPrimeWrapperBool*>(dataStore.GetRawBufferData(EntityDataRequestSyncAnimationID[indexDataID]));
			float realTime = World->GetRealTimeSeconds();

			for (int i = 0; i < dataStore.GetActiveEntityCount(); ++i)
			{
				if (entityDataRequestSyncNotify[i].ValueBool)
				{
					entityDataRequestSyncNotify[i].ValueBool = false;
					SyncAnimation(entityDataId[i].ValueInt16, indexDataID,  
					entityDataAnimState[i].ValueInt8, entityDataStartTimeAnim[i].ValueFloat, entityDataPreviousFrameAnim[i].ValueFloat, realTime);
				}
				
				UpdateAnimation(entityDataId[i].ValueInt16, indexDataID, entityDataState[i].ValueInt8, 
					entityDataAnimState[i].ValueInt8, entityDataStartTimeAnim[i].ValueFloat, entityDataPreviousFrameAnim[i].ValueFloat, realTime);
				
				int32 targetCellId = SpatialGridSystems.GetSpatialGridSystemsCellID(entityDataTargetLocation[i]);
			
				if (entityDataCellId[i].ValueInt32 != targetCellId && !RuntimeAnimationMapping[indexDataID].AnimationForceNotify[entityDataState[i].ValueInt8]) continue;
			
				PlayAnimation(entityDataAnimState[i].ValueInt8, entityDataStartTimeAnim[i].ValueFloat, indexDataID,
					entityDataPreviousFrameAnim[i].ValueFloat, entityDataRequestAnimationNotify[i].ValueInt32, realTime);
			}
			
			indexDataID++;
		}
	}
};