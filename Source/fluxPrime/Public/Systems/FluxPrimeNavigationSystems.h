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
			FVector location = FVector(members.CrowdsLocation[i].X, members.CrowdsLocation[i].Y, 0);
			int8 indexNavigationPath = members.CrowdsIndexNavigationPath[i];
			if (FVector::DistXY(location, members.CrowdsNavigationPath[i].LocationPaths[indexNavigationPath++]) < 50)
				members.CrowdsIndexNavigationPath[i]++;
			
			float dist = FVector::DistSquaredXY(location, members.CrowdsNavigationPath[i].LocationPaths[members.CrowdsTotalNavigationPath[i]]);
			if (dist < 2500.0f) continue;
			
			if (members.CrowdsIndexNavigationPath[i] == members.CrowdsTotalNavigationPath[i])
			{
				TArray<FVector> path = GetNavigationPath(World, members.CrowdsLocation[i], members.CrowdsTargetLocation[i]);
				int8 total = FMath::Min(path.Num() - 1, 8);
    
				for (int8 j = 0; j < total; ++j)
				{
					path[j+1].Z = 0;
					members.CrowdsNavigationPath[i].LocationPaths[j] = path[j+1];
					
					DrawDebugLine(World, path[j+1] + (FVector::UpVector * 1000), path[j+1], FColor::Red, true, 10.0f, 0, 2.0f);
				}
				
				members.CrowdsIndexNavigationPath[i] = 0;
				members.CrowdsTotalNavigationPath[i] = total;
			}
		}
	}
};
