#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeNotifyDeadSystems.generated.h"

USTRUCT()
struct FFluxPrimeNotifyDeadSystemsContext
{
	GENERATED_BODY()
	
	TArray<FVector>* locationCrowds = nullptr;
	TArray<bool>* requestBackPoolCrowds = nullptr;
	TArray<EFluxPrimeCrowdAnimationNotify>* requestNotifyCrowds = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeNotifyDeadSystems : public FFluxPrimeNotifyBaseSystems
{
	GENERATED_BODY()
	
private:
	TArray<FVector>* LocationCrowds = nullptr;
	TArray<bool>* RequestBackPoolCrowds = nullptr;
	
public:
	void InitializeNotifyDeadSystems(FFluxPrimeNotifyDeadSystemsContext context)
	{
		check(context.locationCrowds);
		check(context.requestBackPoolCrowds);
		check(context.requestNotifyCrowds);
		
		RequestNotifyCrowds = context.requestNotifyCrowds;
		LocationCrowds = context.locationCrowds;
		RequestBackPoolCrowds = context.requestBackPoolCrowds;
	}
	
	void ExecuteNotify(const uint16 indexMember)
	{
		if ((*RequestNotifyCrowds)[indexMember] != EFluxPrimeCrowdAnimationNotify::NotifyDead) return;
		
		(*RequestBackPoolCrowds)[indexMember] = true;
		(*LocationCrowds)[indexMember] = FVector::DownVector * 1000.0f;
	}
};
