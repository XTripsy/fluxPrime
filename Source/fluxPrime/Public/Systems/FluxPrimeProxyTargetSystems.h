#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeNavigationSystems.h"
#include "Kismet/GameplayStatics.h"
#include "FluxPrimeBaseSystems.h"
#include "FluxPrimeProxyTargetSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeProxyTargetSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()
	
private:
	FTimerHandle TimerHandle = FTimerHandle();
	TStaticArray<FFluxPrimeCrowds, 2>* Crowds = nullptr;
	FFluxPrimeNavigationSystems* NavigationSystems = nullptr;
	uint16* ActiveMember = nullptr;
	
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
private:
	// perlu di ubah
	void UpdateProxyTargetLocation()
	{
		FVector targetLocation = UGameplayStatics::GetPlayerPawn(World, 0)->GetActorLocation();
		
		for (int i = 0; i < *ActiveMember; ++i)
		{
			(*Crowds)[0].CrowdsTargetLocation[i] = targetLocation;
			(*Crowds)[1].CrowdsTargetLocation[i] = targetLocation;
			
			//TArray<FVector> path = NavigationSystems->GetNavigationPath(World, Crowds->CrowdsLocation[i], targetLocation);
			TArray<FVector> path;
			if (!NavigationSystems->CalculatePath((*Crowds)[0].CrowdsLocation[i], targetLocation, path)) continue;
			
			int8 total = FMath::Min(path.Num() - 1, FluxConfig::NavigationArrayCount);
			
			for (int8 j = 0; j < total; ++j)
			{
				path[j+1].Z = 0;
				(*Crowds)[0].CrowdsNavigationPath[i].LocationPaths[j] = path[j+1];
				(*Crowds)[1].CrowdsNavigationPath[i].LocationPaths[j] = path[j+1];
			}
			
			(*Crowds)[0].CrowdsIndexNavigationPath[i] = 0;
			(*Crowds)[1].CrowdsIndexNavigationPath[i] = 0;
			(*Crowds)[0].CrowdsTotalNavigationPath[i] = total;
			(*Crowds)[1].CrowdsTotalNavigationPath[i] = total;
			
			UE_LOG(LogTemp, Error, TEXT("GENERATE"));
		}
	}
	
public:
	void InitializedProxyTargetSystems(TObjectPtr<UWorld> world, TStaticArray<FFluxPrimeCrowds, 2>* crowds, uint16* activeMember, FFluxPrimeNavigationSystems* navigationSystems)
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
