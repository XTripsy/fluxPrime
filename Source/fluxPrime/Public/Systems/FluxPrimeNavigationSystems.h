#pragma once

#include "CoreMinimal.h"
#include "Cores/FluxPrimeStruct.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "FluxPrimeNavigationSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeNavigationSystems
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
	UPROPERTY()
	bool IsDebug = false;
	
private:
	void ShowDebug(TObjectPtr<UWorld> world, TArray<FVector> path)
	{
		for (int i = 0; i < path.Num(); ++i)
		{
			DrawDebugSphere(
				World,
				path[i],
				50.0f,
				8,
				FColor::Green,
				false,
				-1.0f,
				0,
				3.0f
			);
		}
	}
	
	void ShowDebugText(FFluxPrimeCrowds& members, int32 indexMembers)
	{
		FVector textLocation = members.CrowdsLocation[indexMembers] + (FVector::UpVector * FluxConfig::DebugLocationNavigation);
		FString debugData = FString::Printf(TEXT("Navigation Index Target: %d \n Navigation Total Path: %d"), members.CrowdsIndexNavigationPath[indexMembers], members.CrowdsTotalNavigationPath[indexMembers]);
		
		DrawDebugString(
			World,
			textLocation,
			debugData,
			nullptr,
			FColor::Green,
			0.0f,
			false,
			FluxConfig::DebugScaleFont
		);
	}
	
public:
	void InitializedNavigationSystems(bool isDebug)
	{
		IsDebug = isDebug;
	}
	
	TArray<FVector> GetNavigationPath(TObjectPtr<UWorld> world, FVector start, FVector end)
	{
		World = world;
		UNavigationSystemV1* navigation = UNavigationSystemV1::GetCurrent(world);

		UNavigationPath* path = navigation->FindPathToLocationSynchronously(world, start, end);
		if (IsDebug) ShowDebug(world, path->PathPoints); 
		return path->PathPoints;
	}
	
	void UpdateNavigationSystems(FFluxPrimeCrowds& members, int32 memberActive)
	{
		for (int i = 0; i < memberActive; ++i)
		{
			FVector location = FVector(members.CrowdsLocation[i].X, members.CrowdsLocation[i].Y, 0);
			int8 indexNavigationPath = members.CrowdsIndexNavigationPath[i];
			if (FVector::DistXY(location, members.CrowdsNavigationPath[i].LocationPaths[indexNavigationPath++]) < 50)
				members.CrowdsIndexNavigationPath[i]++;
			
			float dist = FVector::DistSquaredXY(location, members.CrowdsNavigationPath[i].LocationPaths[members.CrowdsTotalNavigationPath[i]]);
			if (dist < 2500.0f) continue;
			
			if (IsDebug) ShowDebugText(members, i);
			
			if (members.CrowdsIndexNavigationPath[i] == members.CrowdsTotalNavigationPath[i])
			{
				TArray<FVector> path = GetNavigationPath(World, members.CrowdsLocation[i], members.CrowdsTargetLocation[i]);
				int8 total = FMath::Min(path.Num() - 1, FluxConfig::NavigationArrayCount);
    
				for (int8 j = 0; j < total; ++j)
				{
					path[j+1].Z = 0;
					members.CrowdsNavigationPath[i].LocationPaths[j] = path[j+1];
				}
				
				members.CrowdsIndexNavigationPath[i] = 0;
				members.CrowdsTotalNavigationPath[i] = total;
			}
		}
	}
	
	void EndPlayNavigationSystems()
	{
		World = nullptr;
	}
};
