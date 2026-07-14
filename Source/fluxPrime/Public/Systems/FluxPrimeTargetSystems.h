#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystems.h"
#include "Cores/FluxPrimeStruct.h"
#include "Kismet/GameplayStatics.h"
#include "FluxPrimeTargetSystems.generated.h"

USTRUCT()
struct FFluxPrimeTargetSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<UWorld> world = nullptr;
	
	UPROPERTY()
	bool isDebug = false;
	
	TArray<FVector>* locationCrowds = nullptr;
	TArray<FVector>* locationTargetCrowds = nullptr;
	TArray<bool>* requestNavigationPathCrowds = nullptr;
	uint16* memberActive = nullptr;
	
};

USTRUCT(BlueprintType)
struct FFluxPrimeTargetSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
	UPROPERTY()
	bool IsDebug = false;
	
	TArray<FVector>* LocationCrowds = nullptr;
	TArray<FVector>* LocationTargetCrowds = nullptr;
	TArray<bool>* RequestNavigationPathCrowds = nullptr;
	uint16* MemberActive = nullptr;
	
	FVector LastPositionTarget, CurrentPositionTarget;
	
private:
	void ShowDebug()
	{
		DrawDebugSphere(
			World,
			LastPositionTarget,
			50.0f,
			8,
			FColor::Red,
			false,
			2.5f,
			0,
			3.0f
		);
	}
	
public:
	void InitializeTargetSystems(FFluxPrimeTargetSystemsContext context)
	{
		check(context.world);
		check(context.locationCrowds);
		check(context.locationTargetCrowds);
		check(context.requestNavigationPathCrowds);
		check(context.memberActive);
		
		World = context.world;
		IsDebug = context.isDebug;
		LocationCrowds = context.locationCrowds;
		LocationTargetCrowds = context.locationTargetCrowds;
		RequestNavigationPathCrowds = context.requestNavigationPathCrowds;
		MemberActive = context.memberActive;
		
		FVector targetLocation = UGameplayStatics::GetPlayerPawn(World, 0)->GetActorLocation();
		LastPositionTarget = targetLocation;
		CurrentPositionTarget = targetLocation;
	}
	
	// ini masih error
	void UpdateTargetSystems()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Target_Systems);
		
		auto& locationCrowds = *LocationCrowds;
		auto& requestCrowds = *RequestNavigationPathCrowds;
		auto& targetLocationCrowds = *LocationTargetCrowds;
		
		CurrentPositionTarget = UGameplayStatics::GetPlayerPawn(World, 0)->GetActorLocation();
		
		TArray<uint8> queue;
		for (int i = 0; i < *MemberActive; ++i)
		{
			if (FVector::DistSquaredXY(locationCrowds[i], CurrentPositionTarget) < 1000000 &&
				FVector::DistSquaredXY(LastPositionTarget, CurrentPositionTarget) > 10000 &&
				!requestCrowds[i])
			{
				requestCrowds[i] = true;
				targetLocationCrowds[i] = CurrentPositionTarget;
			}
			
			queue.Add(i);
		}
		
		if (FVector::DistSquaredXY(LastPositionTarget, CurrentPositionTarget) > 1000000)
		{
			for (auto& pair : queue)
			{
				if (requestCrowds[pair]) continue;
				
				requestCrowds[pair] = true;
				targetLocationCrowds[pair] = CurrentPositionTarget;
			}
			
			if (IsDebug) ShowDebug();
			LastPositionTarget = CurrentPositionTarget;
			if (IsDebug) ShowDebug();
		}
	}
};
