// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/Components/FluxPrimeCrowdsAnimationComponent.h"

#include "Systems/FluxPrimeAnimationSystems.h"

/*
void UFluxPrimeCrowdsAnimationComponent::Initialize(FFluxPrimeAnimationSystems& animationSystems, TArray<TObjectPtr<UInstancedStaticMeshComponent>> crowdsComponents)
{
	SetIsReplicated(true);
	
	animationSystems.OnAttackNotify.BindUObject(this, &UFluxPrimeCrowdsAnimationComponent::OnAnimationAttackNotify);
	animationSystems.OnSpawnVFXNotify.BindUObject(this, &UFluxPrimeCrowdsAnimationComponent::OnAnimationVFXNotify);
	animationSystems.OnSpawnSFXNotify.BindUObject(this, &UFluxPrimeCrowdsAnimationComponent::OnAnimationSFXNotify);
	
	CrowdsComponents = crowdsComponents;
}
*/

/*void UFluxPrimeCrowdsAnimationComponent::Initialize(FFluxPrimeCrowdsAnimationComponentContext context)
{
	SetIsReplicated(true);
	
	context.animationSystems->OnAttackNotify.BindUObject(this, &UFluxPrimeCrowdsAnimationComponent::OnAnimationAttackNotify);
	context.animationSystems->OnSpawnVFXNotify.BindUObject(this, &UFluxPrimeCrowdsAnimationComponent::OnAnimationVFXNotify);
	context.animationSystems->OnSpawnSFXNotify.BindUObject(this, &UFluxPrimeCrowdsAnimationComponent::OnAnimationSFXNotify);
	
	CrowdsComponents = context.crowdsComponents;
}*/

void UFluxPrimeCrowdsAnimationComponent::OnAnimationAttackNotify(int32 id)
{
	
}

void UFluxPrimeCrowdsAnimationComponent::OnAnimationVFXNotify(int32 id)
{
	
}

void UFluxPrimeCrowdsAnimationComponent::OnAnimationSFXNotify(int32 id)
{
	
}

void UFluxPrimeCrowdsAnimationComponent::SwitchAnimation(const FInstancedStruct& payload)
{
	const FFluxPrimeOnSwictAnimationPayload* data = payload.GetPtr<FFluxPrimeOnSwictAnimationPayload>();
	if (!data) return;
	
	uint32 id = data->IdPayload;
	uint32 type = data->TypePayload;
	
	if (!CrowdsComponents.IsValidIndex(type)) return;
	
	/*CrowdsComponents[type]->SetCustomDataValue(
		id,
		0,
		GetWorld()->GetRealTimeSeconds(),
		false
		);
	
	CrowdsComponents[type]->SetCustomDataValue(
		id,
		1,
		data->StartFrameAnimationPayload,
		false
		);
	
	CrowdsComponents[type]->SetCustomDataValue(
		id,
		2,
		data->EndFrameAnimationPayload,
		false
		);*/
}
