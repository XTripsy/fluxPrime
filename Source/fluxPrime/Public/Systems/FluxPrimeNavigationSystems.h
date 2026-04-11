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
	
	UPROPERTY()
	TObjectPtr<UWorld> World;
	
	TArray<FVector> GetNavigationPath(TObjectPtr<UWorld> world, FVector start, FVector end)
	{
		World = world;
		UNavigationSystemV1* navigation = UNavigationSystemV1::GetCurrent(world);

		UNavigationPath* path = navigation->FindPathToLocationSynchronously(world, start, end);
		return path->PathPoints;
	}
	
	void UpdateNavigationSystems(FFluxPrimeCrowds& members, int32 memberActive)
	{
		for (int i = 0; i < memberActive; ++i)
		{
			if (members.CrowdsIndexNavigationPath[i] == members.CrowdsTotalNavigationPath[i])
			{
				TArray<FVector> path = GetNavigationPath(World, members.CrowdsLocation[i], members.CrowdsTargetLocation[i]);
				int8 total = FMath::Min(path.Num() - 1, 8);
    
				for (int8 j = 0; j < total; ++j)
				{
					members.CrowdsNavigationPath[i].LocationPaths[j] = path[j+1];
				}
				
				members.CrowdsIndexNavigationPath[i] = 0;
				members.CrowdsTotalNavigationPath[i] = path.Num()-1;
			}
		}
	}
};
