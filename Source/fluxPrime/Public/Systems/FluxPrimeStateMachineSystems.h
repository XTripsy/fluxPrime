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
	
	TArray<EFluxPrimeCrowdState>* stateCrowds = nullptr;
	TArray<FVector>* locationCrowds = nullptr;
	TArray<FVector>* targetLocationCrowds = nullptr;
	TArray<FVector>* velocityCrowds = nullptr;
	TArray<bool>* conditionCrowds = nullptr;
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
	
	TArray<EFluxPrimeCrowdState>* StateCrowds = nullptr;
	TArray<FVector>* LocationCrowds = nullptr;
	TArray<FVector>* TargetLocationCrowds = nullptr;
	TArray<FVector>* VelocityCrowds = nullptr;
	TArray<bool>* ConditionCrowds = nullptr;
	uint16* MemberActive = nullptr;
	
private:
	static bool CheckCondition(const FFluxPrimeCrowdAnimationStateTransition& Transition, 
		const TArray<FVector>& locationCrowds, const TArray<FVector>& targetLocationCrowds,
		const TArray<FVector>& velocityCrowds, const TArray<bool>& conditionCrowds, int32 Index)
	{
		switch (Transition.ConditionType)
		{
		case EFluxPrimeCrowdAnimationTransitionType::IsAlive:
			return !conditionCrowds[Index];
		case EFluxPrimeCrowdAnimationTransitionType::VelocityAbove:
			return velocityCrowds[Index].SizeSquared() > Transition.Value;
		case EFluxPrimeCrowdAnimationTransitionType::TargetInRange:
			return FVector::Dist2D(locationCrowds[Index], targetLocationCrowds[Index]) < Transition.Value;
		case EFluxPrimeCrowdAnimationTransitionType::TargetOutRange:
			return FVector::Dist2D(locationCrowds[Index], targetLocationCrowds[Index]) > Transition.Value;
		default:
			return false;
		}
	}
	
public:
	void InitializeStateMachineSystems(FFluxPrimeStateMachineSystemsContext context)
	{
		check(context.memberActive);
		check(context.locationCrowds);
		check(context.targetLocationCrowds);
		check(context.velocityCrowds);
		check(context.stateCrowds);
		check(context.conditionCrowds);
		
		IsDebug = context.isDebug;
		LocationCrowds = context.locationCrowds;
		TargetLocationCrowds = context.targetLocationCrowds;
		VelocityCrowds = context.velocityCrowds;
		StateCrowds = context.stateCrowds;
		ConditionCrowds = context.conditionCrowds;
		MemberActive = context.memberActive;
		
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
	
	static void ChangeCrowdState(TArray<EFluxPrimeCrowdState>& state, const int32 memberIndex, const EFluxPrimeCrowdState newState)
	{
		state[memberIndex] = newState;
	}
	
	void UpdateStateMachineSystems()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_FSM_Systems);
		
		auto& stateCrowds = *StateCrowds;
		auto& locationCrowds = *LocationCrowds;
		auto& targetCrowds = *TargetLocationCrowds;
		auto& velocityCrowds = *VelocityCrowds;
		auto& conditionCrowds = *ConditionCrowds;
		
		for (int32 i = 0; i < *MemberActive; ++i)
		{
			EFluxPrimeCrowdState CurrentState = stateCrowds[i];

			for (const auto& pair : Transitions)
			{
				if (pair.From != CurrentState) continue;
				if (!CheckCondition(pair, locationCrowds, targetCrowds, velocityCrowds, conditionCrowds, i)) continue;
				
				ChangeCrowdState(stateCrowds, i, pair.To);
				break;
			}
		}
	}
};
