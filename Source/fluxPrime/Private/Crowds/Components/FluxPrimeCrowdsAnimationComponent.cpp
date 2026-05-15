// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/Components/FluxPrimeCrowdsAnimationComponent.h"

#include "Systems/FluxPrimeAnimationSystems.h"


void UFluxPrimeCrowdsAnimationComponent::Initialize(FFluxPrimeAnimationSystems& animationSystems)
{
	animationSystems.OnAttackNotify.BindUObject(this, &UFluxPrimeCrowdsAnimationComponent::OnAnimationAttackNotify);
	animationSystems.OnSpawnVFXNotify.BindUObject(this, &UFluxPrimeCrowdsAnimationComponent::OnAnimationVFXNotify);
	animationSystems.OnSpawnSFXNotify.BindUObject(this, &UFluxPrimeCrowdsAnimationComponent::OnAnimationSFXNotify);
}

void UFluxPrimeCrowdsAnimationComponent::OnAnimationAttackNotify(int32 id)
{
	
}

void UFluxPrimeCrowdsAnimationComponent::OnAnimationVFXNotify(int32 id)
{
	
}

void UFluxPrimeCrowdsAnimationComponent::OnAnimationSFXNotify(int32 id)
{
	
}
