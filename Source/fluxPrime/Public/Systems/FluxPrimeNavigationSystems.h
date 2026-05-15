#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystems.h"
#include "Cores/FluxPrimeStruct.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavigationPath.h"
#include "FluxPrimeNavigationSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeNavigationSystems : public FFluxPrimeBaseSystems
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
	UPROPERTY()
	bool IsDebug = false;
	
private:
	void ShowDebug(TArray<FVector> path)
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
	void InitializedNavigationSystems(bool isDebug, TObjectPtr<UWorld> world)
	{
		World = world;
		IsDebug = isDebug;
	}
	
	bool CalculatePath(const FVector& start, const FVector& end, TArray<FVector>& outPathPoints)
	{
		UNavigationSystemV1* navSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
		if (!navSys) return false;

		ANavigationData* navData = navSys->GetDefaultNavDataInstance();
		if (!navData) return false;

		FPathFindingQuery query(World, *navData, start, end);
		FPathFindingResult result = navData->FindPath(navSys->GetDefaultSupportedAgent(), query);

		if (result.IsSuccessful() && result.Path.IsValid())
		{
			const TArray<FNavPathPoint>& pathPoints = result.Path->GetPathPoints();
			outPathPoints.Reset(pathPoints.Num()); 
        
			for (const FNavPathPoint& point : pathPoints)
			{
				outPathPoints.Add(point.Location);
			}
			
			if (IsDebug) ShowDebug(outPathPoints); 
			return true;
		}

		return false;
	}
	
	TArray<FVector> GetNavigationPath(TObjectPtr<UWorld> world, FVector start, FVector end)
	{
		World = world;
		UNavigationSystemV1* navigation = UNavigationSystemV1::GetCurrent(world);

		UNavigationPath* path = navigation->FindPathToLocationSynchronously(world, start, end);
		if (IsDebug) ShowDebug(path->PathPoints); 
		return path->PathPoints;
	}
	
	//void UpdateNavigationSystems(TStaticArray<FFluxPrimeCrowds, 2>& members, int32 memberActive)
	void UpdateNavigationSystems(FFluxPrimeCrowds& members, int32 memberActive)
	{
		for (int i = 0; i < memberActive; ++i)
		{
			//FVector location = FVector(members[0].CrowdsLocation[i].X, members[0].CrowdsLocation[i].Y, 0);
			FVector location = FVector(members.CrowdsLocation[i].X, members.CrowdsLocation[i].Y, 0);
			//int8 indexNavigationPath = members[0].CrowdsIndexNavigationPath[i];
			int8 indexNavigationPath = members.CrowdsIndexNavigationPath[i];
			UE_LOG(LogTemp, Error, TEXT("CEK DISTANCE"));
			
			//if (!members[0].CrowdsNavigationPath[i].LocationPaths->IsValidIndex(indexNavigationPath+1))
			if (!members.CrowdsNavigationPath[i].LocationPaths->IsValidIndex(indexNavigationPath+1))
			{
				UE_LOG(LogTemp, Warning, TEXT("NOT VALID"));
				continue;
			}
			
			//if (FVector::DistXY(location, members[0].CrowdsNavigationPath[i].LocationPaths[indexNavigationPath]) < 50)
			if (FVector::DistXY(location, members.CrowdsNavigationPath[i].LocationPaths[indexNavigationPath]) < 50)
			{
				/*members[0].CrowdsIndexNavigationPath[i]++;
				members[1].CrowdsIndexNavigationPath[i]++;*/
				members.CrowdsIndexNavigationPath[i]++;
				UE_LOG(LogTemp, Warning, TEXT("INDEX ADD"));
			}
			
			//float dist = FVector::DistSquaredXY(location, members[0].CrowdsNavigationPath[i].LocationPaths[members[0].CrowdsTotalNavigationPath[i]]);
			float dist = FVector::DistSquaredXY(location, members.CrowdsNavigationPath[i].LocationPaths[members.CrowdsTotalNavigationPath[i]]);
			if (dist < 2500.0f) continue;
			
			//if (IsDebug) ShowDebugText(members[0], i);
			if (IsDebug) ShowDebugText(members, i);
			
			//if (members[0].CrowdsIndexNavigationPath[i] == members[0].CrowdsTotalNavigationPath[i])
			if (members.CrowdsIndexNavigationPath[i] == members.CrowdsTotalNavigationPath[i])
			{
				//TArray<FVector> path = GetNavigationPath(World, members.CrowdsLocation[i], members.CrowdsTargetLocation[i]);
				TArray<FVector> path;
				//if (!CalculatePath( members[0].CrowdsLocation[i], members[0].CrowdsTargetLocation[i], path)) continue;
				if (!CalculatePath( members.CrowdsLocation[i], members.CrowdsTargetLocation[i], path)) continue;
				
				int8 total = FMath::Min(path.Num() - 1, FluxConfig::NavigationArrayCount);
    
				for (int8 j = 0; j < total; ++j)
				{
					path[j+1].Z = 0;
					/*members[0].CrowdsNavigationPath[i].LocationPaths[j] = path[j+1];
					members[1].CrowdsNavigationPath[i].LocationPaths[j] = path[j+1];*/
					members.CrowdsNavigationPath[i].LocationPaths[j] = path[j+1];
				}
				
				members.CrowdsIndexNavigationPath[i] = 0;
				members.CrowdsTotalNavigationPath[i] = total;
				/*members[0].CrowdsIndexNavigationPath[i] = 0;
				members[1].CrowdsIndexNavigationPath[i] = 0;
				members[0].CrowdsTotalNavigationPath[i] = total;
				members[1].CrowdsTotalNavigationPath[i] = total;*/
			}
		}
	}
	
	void EndPlayNavigationSystems()
	{
		World = nullptr;
	}
};
