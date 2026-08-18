#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeNotifyBaseSystem.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeNotifyAbilitySystem.generated.h"

USTRUCT()
struct FFluxPrimeNotifyAbilitySystemsContext
{
	GENERATED_BODY()
	
	TArray<bool>* requestAbilityCrowds = nullptr;
	TArray<EFluxPrimeCrowdAnimationNotify>* requestNotifyCrowds = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeNotifyAbilitySystem : public FFluxPrimeNotifyBaseSystem
{
	GENERATED_BODY()
	
private:
	TArray<bool>* RequestAbilityCrowds = nullptr;
	
public:
	void InitializeNotifyAttackSystems(FFluxPrimeNotifyAbilitySystemsContext context)
	{
		check(context.requestAbilityCrowds);
		check(context.requestNotifyCrowds);
		
		RequestAbilityCrowds = context.requestAbilityCrowds;
		RequestNotifyCrowds = context.requestNotifyCrowds;
	}
	
	void ExecuteNotify(const uint16 indexMember)
	{
		if ((*RequestNotifyCrowds)[indexMember] != EFluxPrimeCrowdAnimationNotify::NotifyAbility) return;
		
		(*RequestAbilityCrowds)[indexMember] = true;
		(*RequestNotifyCrowds)[indexMember] = EFluxPrimeCrowdAnimationNotify::NotifyNone;
		UE_LOG(LogTemp, Log, TEXT("NOTIFY:: ATTACK"));
	}
};
