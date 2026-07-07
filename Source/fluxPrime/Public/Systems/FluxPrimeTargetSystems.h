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
	
	//TStaticArray<FFluxPrimeCrowds, 2>* members = nullptr;
	FFluxPrimeCrowds* members = nullptr;
	//int8* dataReadIndex = nullptr;
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
	
	//TStaticArray<FFluxPrimeCrowds, 2>* Members = nullptr;
	FFluxPrimeCrowds* Members = nullptr;
	//int8* DataReadIndex = nullptr;
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
		check(context.members);
		check(context.memberActive);
		//check(context.dataReadIndex);
		
		World = context.world;
		IsDebug = context.isDebug;
		Members = context.members;
		MemberActive = context.memberActive;
		//DataReadIndex = context.dataReadIndex;
		
		FVector targetLocation = UGameplayStatics::GetPlayerPawn(World, 0)->GetActorLocation();
		LastPositionTarget = targetLocation;
		CurrentPositionTarget = targetLocation;
	}
	
	// ini masih error
	void UpdateTargetSystems()
	{
		//auto& members = (*Members)[*DataReadIndex];
		auto& members = *Members;
		CurrentPositionTarget = UGameplayStatics::GetPlayerPawn(World, 0)->GetActorLocation();
		
		TArray<uint8> queue;
		for (int i = 0; i < *MemberActive; ++i)
		{
			if (FVector::DistSquaredXY(members.CrowdsLocation[i], CurrentPositionTarget) < 1000000 &&
				FVector::DistSquaredXY(LastPositionTarget, CurrentPositionTarget) > 10000 &&
				!members.CrowdsRequestNavigationPath[i])
			{
				members.CrowdsRequestNavigationPath[i] = true;
				members.CrowdsTargetLocation[i] = CurrentPositionTarget;
			}
			
			queue.Add(i);
		}
		
		if (FVector::DistSquaredXY(LastPositionTarget, CurrentPositionTarget) > 1000000)
		{
			for (auto& pair : queue)
			{
				if (members.CrowdsRequestNavigationPath[pair]) continue;
				
				members.CrowdsRequestNavigationPath[pair] = true;
				members.CrowdsTargetLocation[pair] = CurrentPositionTarget;
			}
			
			if (IsDebug) ShowDebug();
			LastPositionTarget = CurrentPositionTarget;
			if (IsDebug) ShowDebug();
		}
	}
};
