#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeNavigationSystems.h"
#include "Kismet/GameplayStatics.h"
#include "FluxPrimeProxyTargetSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeProxyTargetSystems
{
	GENERATED_BODY()
	
private:
	FTimerHandle TimerHandle = FTimerHandle();
	FFluxPrimeCrowds* Crowds = nullptr;
	FFluxPrimeNavigationSystems* NavigationSystems = nullptr;
	int32* ActiveMember = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
private:
	void UpdateProxyTargetLocation()
	{
		FVector targetLocation = UGameplayStatics::GetPlayerPawn(World, 0)->GetActorLocation();
		
		for (int i = 0; i < *ActiveMember; ++i)
		{
			Crowds->CrowdsTargetLocation[i] = targetLocation;
			
			TArray<FVector> path = NavigationSystems->GetNavigationPath(World, Crowds->CrowdsLocation[i], targetLocation);
			int8 total = FMath::Min(path.Num() - 1, FluxConfig::NavigationArrayCount);
		
			Crowds->CrowdsIndexNavigationPath[i] = 0;
			Crowds->CrowdsTotalNavigationPath[i] = total;
			
			for (int8 j = 0; j < total; ++j)
			{
				path[j+1].Z = 0;
				Crowds->CrowdsNavigationPath[i].LocationPaths[j] = path[j+1];
			}
		}
	}
	
public:
	void InitializedProxyTargetSystems(TObjectPtr<UWorld> world, FFluxPrimeCrowds* crowds, int32* activeMember, FFluxPrimeNavigationSystems* navigationSystems)
	{
		World = world;
		Crowds = crowds;
		ActiveMember = activeMember;
		NavigationSystems = navigationSystems;
		
		FTimerDelegate function = FTimerDelegate::CreateRaw(this, &FFluxPrimeProxyTargetSystems::UpdateProxyTargetLocation);
		World->GetTimerManager().SetTimer(
			TimerHandle,
			function,
			1.0f, true);
	}
	
	void EndPlayProxyTargetSystems()
	{
		if (TimerHandle.IsValid()) World->GetTimerManager().ClearTimer(TimerHandle);
		World = nullptr;
		NavigationSystems = nullptr;
	}
};
