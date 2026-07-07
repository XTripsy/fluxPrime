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
	
	FFluxPrimeCrowds* members = nullptr;
	uint16* memberActive = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeHealthSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()
	
private:
	FFluxPrimeCrowds* Members = nullptr;
	uint16* MemberActive = nullptr;
	
public:
	void InitializeHealthSystems(FFluxPrimeHealthSystemsContext context)
	{
		check(context.members);
		check(context.memberActive);
		
		Members = context.members;
		MemberActive = context.memberActive;
	}
	
	void OnHitHealthSystems(uint32 indexMember)
	{
		FFluxPrimeCrowds& member = *Members;
		member.CrowdsHealth[indexMember]--;
		UE_LOG(LogTemp, Log, TEXT("HEALTH SYSTEMS:: DAMAGE %d"), member.CrowdsHealth[indexMember]);
	}
	
	void UpdateHealthSystems()
	{
		FFluxPrimeCrowds& member = *Members;

		for (int i = 0; i < *MemberActive; ++i)
		{
			if (member.CrowdsCondition[i] && member.CrowdsHealth[i] <= 0) member.CrowdsCondition[i] = false;
		}
	}
};