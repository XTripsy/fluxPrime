#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystems.h"
#include "Cores/FluxPrimeEnum.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeStateMachineSystems.generated.h"

UENUM(BlueprintType)
enum class EFluxPrimeCrowdAnimationTransitionType : uint8
{
	IsAlive,
	VelocityAbove,
	VelocityBelow,
	TargetInRange,
	TargetOutRange,
	AnimationFinished
};

USTRUCT()
struct FFluxPrimeStateMachineSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool isDebug;
	
	//TStaticArray<FFluxPrimeCrowds, 2>* members = nullptr;
	FFluxPrimeCrowds* members = nullptr;
	//int8* dataReadIndex = nullptr;
	uint16* memberActive = nullptr;
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
	
	UPROPERTY()
	bool IsDebug = false;
	
	//TStaticArray<FFluxPrimeCrowds, 2>* Members = nullptr;
	FFluxPrimeCrowds* Members = nullptr;
	//int8* DataReadIndex = nullptr;
	uint16* MemberActive = nullptr;
	
private:
	static bool CheckCondition(const FFluxPrimeCrowdAnimationStateTransition& Transition, const FFluxPrimeCrowds& Members, int32 Index)
	{
		switch (Transition.ConditionType)
		{
		case EFluxPrimeCrowdAnimationTransitionType::IsAlive:
			return !Members.CrowdsCondition[Index];
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
	void InitializeStateMachineSystems(FFluxPrimeStateMachineSystemsContext context)
	{
		check(context.members);
		check(context.memberActive);
		//check(context.dataReadIndex);
		
		IsDebug = context.isDebug;
		Members = context.members;
		MemberActive = context.memberActive;
		//DataReadIndex = context.dataReadIndex;
		
		Transitions.Add({
			EFluxPrimeCrowdState::StateAbility,
			EFluxPrimeCrowdState::StateDead,
			EFluxPrimeCrowdAnimationTransitionType::IsAlive,
			0
		});
		
		Transitions.Add({
			EFluxPrimeCrowdState::StateWalk,
			EFluxPrimeCrowdState::StateDead,
			EFluxPrimeCrowdAnimationTransitionType::IsAlive,
			0
		});
		
		Transitions.Add({
			EFluxPrimeCrowdState::StateIdle,
			EFluxPrimeCrowdState::StateDead,
			EFluxPrimeCrowdAnimationTransitionType::IsAlive,
			0
		});
		
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
	
	static void ChangeCrowdState(FFluxPrimeCrowds& members, const int32 memberIndex, const EFluxPrimeCrowdState newState)
	{
		members.CrowdsState[memberIndex] = newState;
	}
	
	void UpdateStateMachineSystems()
	{
		//auto& members = (*Members)[*DataReadIndex];
		auto& members = *Members;
		
		for (int32 i = 0; i < *MemberActive; ++i)
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
