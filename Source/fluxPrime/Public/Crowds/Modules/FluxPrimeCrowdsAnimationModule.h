#pragma once

#include "CoreMinimal.h"
#include "Systems/FluxPrimeAnimationSystems.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "FluxPrimeCrowdsAnimationModule.generated.h"

USTRUCT()
struct FFluxPrimeCrowdsAnimationComponentContext
{
	GENERATED_BODY()
	
	FFluxPrimeAnimationSystems* animationSystems = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> world;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* crowdsComponents = nullptr;
};

USTRUCT()
struct FFluxPrimeCrowdsAnimationModule
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
	TArray<TObjectPtr<UInstancedStaticMeshComponent>>* CrowdsComponents = nullptr;
	
public:
	void Initialize(FFluxPrimeCrowdsAnimationComponentContext context)
	{
		/*context.animationSystems->OnAttackNotify.BindUObject(this, &OnAnimationAttackNotify);
		context.animationSystems->OnSpawnVFXNotify.BindUObject(this, &OnAnimationVFXNotify);
		context.animationSystems->OnSpawnSFXNotify.BindUObject(this, &OnAnimationSFXNotify);*/
	
		World = context.world;
		CrowdsComponents = context.crowdsComponents;
	}
	
	void OnAnimationAttackNotify(int32 id)
	{
	
	}

	void OnAnimationVFXNotify(int32 id)
	{
	
	}

	void OnAnimationSFXNotify(int32 id)
	{
	
	}

	void SwitchAnimation(const FInstancedStruct& payload)
	{
		const FFluxPrimeOnSwictAnimationPayload* data = payload.GetPtr<FFluxPrimeOnSwictAnimationPayload>();
		if (!data) return;
	
		uint32 id = data->IdPayload;
		uint32 type = data->TypePayload;
	
		if (!CrowdsComponents->IsValidIndex(type)) return;
	
		// previous
		(*CrowdsComponents)[type]->SetCustomDataValue(
			id,
			0,
			data->PreviousStartTimeAnimationPayload,
			false
			);
	
		(*CrowdsComponents)[type]->SetCustomDataValue(
			id,
			1,
			data->PreviousStartFrameAnimationPayload,
			false
			);
	
		(*CrowdsComponents)[type]->SetCustomDataValue(
			id,
			2,
			data->PreviousEndFrameAnimationPayload,
			false
			);
		
		// current
		(*CrowdsComponents)[type]->SetCustomDataValue(
			id,
			3,
			World->GetRealTimeSeconds(),
			false
			);
	
		(*CrowdsComponents)[type]->SetCustomDataValue(
			id,
			4,
			data->CurrentStartFrameAnimationPayload,
			false
			);
	
		(*CrowdsComponents)[type]->SetCustomDataValue(
			id,
			5,
			data->CurrentEndFrameAnimationPayload,
			false
			);
		
			UE_LOG(LogTemp, Log, TEXT("CHANGE ANIMATION:: prevStart[%d], prevEnd[%d], prevTime[%d}, curStart[%d], curEnd[%d], curTime[%f]"), data->PreviousStartFrameAnimationPayload, data->PreviousEndFrameAnimationPayload, data->PreviousStartTimeAnimationPayload, data->CurrentStartFrameAnimationPayload, data->CurrentEndFrameAnimationPayload, World->GetRealTimeSeconds());
		
	}
};
