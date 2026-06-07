#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystems.h"
#include "Cores/FluxPrimeEnum.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeStateMachineSystems.generated.h"

UENUM(BlueprintType)
enum class EFluxPrimeCrowdAnimationTransitionType : uint8
{
	VelocityAbove,
	VelocityBelow,
	TargetInRange,
	TargetOutRange,
	AnimationFinished
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdAnimationStateTransition
{
	GENERATED_BODY()
	
	EFluxPrimeCrowdState From;
	EFluxPrimeCrowdState To;

	EFluxPrimeCrowdAnimationTransitionType ConditionType;

	float Value;
};

USTRUCT(BlueprintType)
struct FFluxPrimeStateMachineSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TArray<FFluxPrimeCrowdAnimationStateTransition> Transitions;
	
private:
	bool CheckCondition(const FFluxPrimeCrowdAnimationStateTransition& Transition, const FFluxPrimeCrowds& Members, int32 Index)
	{
		switch (Transition.ConditionType)
		{
		case EFluxPrimeCrowdAnimationTransitionType::VelocityAbove:
			return Members.CrowdsVelocity[Index].SizeSquared() > Transition.Value;
		case EFluxPrimeCrowdAnimationTransitionType::TargetInRange:
			return FVector::Dist2D(Members.CrowdsLocation[Index], Members.CrowdsTargetLocation[Index]) < Transition.Value;
		case EFluxPrimeCrowdAnimationTransitionType::TargetOutRange:
			return FVector::Dist2D(Members.CrowdsLocation[Index], Members.CrowdsTargetLocation[Index]) > Transition.Value;
		default:
			return false;
		}
	}
	
public:
	void InitializeStateMachineSystems()
	{
		Transitions.Add({
			EFluxPrimeCrowdState::StateIdle,
			EFluxPrimeCrowdState::StateWalk,
			EFluxPrimeCrowdAnimationTransitionType::VelocityAbove,
			10.f
		});

		Transitions.Add({
			EFluxPrimeCrowdState::StateWalk,
			EFluxPrimeCrowdState::StateAbility,
			EFluxPrimeCrowdAnimationTransitionType::TargetInRange,
			250.f
		});
		
		Transitions.Add({
			EFluxPrimeCrowdState::StateAbility,
			EFluxPrimeCrowdState::StateWalk,
			EFluxPrimeCrowdAnimationTransitionType::TargetOutRange,
			650.f
		});
	}
	
	void ChangeCrowdState(FFluxPrimeCrowds& members, const int32 memberIndex, const EFluxPrimeCrowdState newState)
	{
		members.CrowdsState[memberIndex] = newState;
	}
	
	void UpdateStateMachineSystems(FFluxPrimeCrowds& members, const int32 memberActive)
	{
		for (int32 i = 0; i < memberActive; ++i)
		{
			EFluxPrimeCrowdState CurrentState = members.CrowdsState[i];

			for (const auto& pair : Transitions)
			{
				if (pair.From != CurrentState) continue;
				if (!CheckCondition(pair, members, i)) continue;

				ChangeCrowdState(members, i, pair.To);
				break;
			}
		}
	}
};
