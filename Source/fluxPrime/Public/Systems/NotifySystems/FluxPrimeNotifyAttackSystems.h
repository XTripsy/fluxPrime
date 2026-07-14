#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeNotifyBaseSystems.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeNotifyAttackSystems.generated.h"

USTRUCT()
struct FFluxPrimeNotifyAttackSystemsContext
{
	GENERATED_BODY()
	
	TArray<EFluxPrimeCrowdAnimationNotify>* requestNotifyCrowds = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeNotifyAttackSystems : public FFluxPrimeNotifyBaseSystems
{
	GENERATED_BODY()
	
public:
	void InitializeNotifyAttackSystems(FFluxPrimeNotifyAttackSystemsContext context)
	{
		check(context.requestNotifyCrowds);
		
		RequestNotifyCrowds = context.requestNotifyCrowds;
	}
	
	void ExecuteNotify(const uint16 indexMember)
	{
		if ((*RequestNotifyCrowds)[indexMember] != EFluxPrimeCrowdAnimationNotify::NotifyAttack) return;
		
		UE_LOG(LogTemp, Log, TEXT("NOTIFY:: ATTACK"));
	}
};
