#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "DrawDebugHelpers.h"
#include "FluxPrimeBaseSystem.h"
#include "NotifySystems/FluxPrimeNotifyAbilitySystem.h"
#include "NotifySystems/FluxPrimeNotifyDeadSystem.h"
#include "NotifySystems/FluxPrimeNotifySpawnSFXSystem.h"
#include "NotifySystems/FluxPrimeNotifySpawnVFXSystem.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeAnimationNotifySystem.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationNotifySystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool isDebug = false;
	FFluxPrimeCrowds* crowdsDatas = nullptr;
	uint16* memberActive = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeAnimationNotifySystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	bool IsDebug = false;
	FFluxPrimeCrowds* CrowdsDatas = nullptr;
	uint16* MemberActive = nullptr;
	
	UPROPERTY()
	FFluxPrimeNotifyAbilitySystem NotifyAbilitySystems;
	
	UPROPERTY()
	FFluxPrimeNotifySpawnSFXSystem NotifySpawnSfxSystems;
	
	UPROPERTY()
	FFluxPrimeNotifySpawnVFXSystem NotifySpawnVfxSystems;
	
	UPROPERTY()
	FFluxPrimeNotifyDeadSystem NotifyDeadSystems;
	
public:
	void InitializeAnimationNotifySystems(FFluxPrimeAnimationNotifySystemsContext contex)
	{
		check(contex.crowdsDatas);
		check(contex.memberActive);
		
		IsDebug = contex.isDebug;
		CrowdsDatas = contex.crowdsDatas;
		MemberActive = contex.memberActive;
		
		auto& members = *CrowdsDatas;
		
		{
			FFluxPrimeNotifyAbilitySystemsContext context;
			context.requestAbilityCrowds = &members.CrowdsRequestAbility;
			context.requestNotifyCrowds = &members.CrowdsRequestAnimationNotify; 
			NotifyAbilitySystems.InitializeNotifyAttackSystems(context);
		}
		
		{
			FFluxPrimeNotifyDeadSystemsContext context;
			context.requestNotifyCrowds = &members.CrowdsRequestAnimationNotify; 
			context.requestBackPoolCrowds = &members.CrowdsRequestBackToPool;
			context.locationCrowds = &members.CrowdsLocation;
			NotifyDeadSystems.InitializeNotifyDeadSystems(context);
		}
		
		{
			FFluxPrimeNotifySpawnSFXSystemsContext context;
			context.requestNotifyCrowds = &members.CrowdsRequestAnimationNotify; 
			NotifySpawnSfxSystems.InitializeNotifySpawnSFXSsystems(context);
		}
		
		{
			FFluxPrimeNotifySpawnVFXSystemsContext context;
			context.requestNotifyCrowds = &members.CrowdsRequestAnimationNotify; 
			NotifySpawnVfxSystems.InitializeNotifySpawnVFXSystems(context);
		}
	}
	
	void UpdateAnimationNotifySystems()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Notify_Systems);

		for (int i = 0; i < *MemberActive; ++i)
		{
			NotifyAbilitySystems.ExecuteNotify(i);
			NotifyDeadSystems.ExecuteNotify(i);
			NotifySpawnSfxSystems.ExecuteNotify(i);
			NotifySpawnVfxSystems.ExecuteNotify(i);
		}
	}
};