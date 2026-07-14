#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "DrawDebugHelpers.h"
#include "FluxPrimeBaseSystems.h"
#include "NotifySystems/FluxPrimeNotifyAttackSystems.h"
#include "NotifySystems/FluxPrimeNotifyDeadSystems.h"
#include "NotifySystems/FluxPrimeNotifySpawnSFXSystems.h"
#include "NotifySystems/FluxPrimeNotifySpawnVFXSystems.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeAnimationNotifySystems.generated.h"

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
struct FFluxPrimeAnimationNotifySystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	bool IsDebug = false;
	FFluxPrimeCrowds* CrowdsDatas = nullptr;
	uint16* MemberActive = nullptr;
	
	UPROPERTY()
	FFluxPrimeNotifyAttackSystems NotifyAttackSystems;
	
	UPROPERTY()
	FFluxPrimeNotifySpawnSFXSystems NotifySpawnSfxSystems;
	
	UPROPERTY()
	FFluxPrimeNotifySpawnVFXSystems NotifySpawnVfxSystems;
	
	UPROPERTY()
	FFluxPrimeNotifyDeadSystems NotifyDeadSystems;
	
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
			FFluxPrimeNotifyAttackSystemsContext context;
			context.requestNotifyCrowds = &members.CrowdsRequestAnimationNotify; 
			NotifyAttackSystems.InitializeNotifyAttackSystems(context);
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
			NotifyAttackSystems.ExecuteNotify(i);
			NotifyDeadSystems.ExecuteNotify(i);
			NotifySpawnSfxSystems.ExecuteNotify(i);
			NotifySpawnVfxSystems.ExecuteNotify(i);
		}
	}
};