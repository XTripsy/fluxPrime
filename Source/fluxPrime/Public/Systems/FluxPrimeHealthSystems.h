#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystems.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeHealthSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeHealthSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool isDebug = false;
	
	TArray<bool>* conditionCrowds = nullptr;
	TArray<int8>* healthCrowds = nullptr;
	uint16* memberActive = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeHealthSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()
	
private:
	TArray<bool>* ConditionCrowds = nullptr;
	TArray<int8>* HealthCrowds = nullptr;
	uint16* MemberActive = nullptr;
	
public:
	void InitializeHealthSystems(FFluxPrimeHealthSystemsContext context)
	{
		check(context.conditionCrowds);
		check(context.healthCrowds);
		check(context.memberActive);
		
		ConditionCrowds = context.conditionCrowds;
		HealthCrowds = context.healthCrowds;
		MemberActive = context.memberActive;
	}
	
	void OnHitHealthSystems(uint32 indexMember)
	{
		auto& healthCrowds = *HealthCrowds;
		healthCrowds[indexMember]--;
		UE_LOG(LogTemp, Log, TEXT("HEALTH SYSTEMS:: DAMAGE %d"), healthCrowds[indexMember]);
	}
	
	void UpdateHealthSystems()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Health_Systems);
		
		auto& conditionCrowds = *ConditionCrowds;
		auto& healthCrowds = *HealthCrowds;
		
		for (int i = 0; i < *MemberActive; ++i)
		{
			if (conditionCrowds[i] && healthCrowds[i] <= 0) conditionCrowds[i] = false;
		}
	}
};