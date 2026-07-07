#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystems.h"
#include "FluxPrimeSpatialGridSystems.h"
#include "Cores/FluxPrimeStruct.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavigationPath.h"
#include "FluxPrimeNavigationSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeNavigationSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool isDebug = false;
	
	UPROPERTY(EditAnywhere)
	FFluxPrimeSpatialGridSystemsContext contextSpatialGrid;
};

USTRUCT(BlueprintType)
struct FFluxPrimeNavigationSystems : public FFluxPrimeBaseSystems
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
	
	FFluxPrimeSpatialGridSystems SpatialGridSystems;
	
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
				2.0f,
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
	
	void RegeneratePath(FFluxPrimeCrowds& members, int32 i)
	{
		TArray<FVector> path;
		if (!CalculatePath(members.CrowdsLocation[i], members.CrowdsTargetLocation[i], path)) return;
		
		UE_LOG(LogTemp, Log, TEXT("NAVIGATION SYSTEMS:: REGENERATE SUCCESS"));
		int8 total = FMath::Min(path.Num() - 1, FluxConfig::NavigationArrayCount);
    
		for (int8 j = 0; j < total; ++j)
		{
			path[j+1].Z = 0;
			members.CrowdsNavigationPath[i].LocationPaths[j] = path[j+1];
		}
				
		members.CrowdsIndexNavigationPath[i] = 0;
		members.CrowdsTotalNavigationPath[i] = total;
		members.CrowdsRequestNavigationPath[i] = false;
	}
	
public:
	void InitializedNavigationSystems(FFluxPrimeNavigationSystemsContext context)
	{
		check(context.contextSpatialGrid.members);
		check(context.contextSpatialGrid.memberActive);
		//check(context.contextSpatialGrid.dataReadIndex);
		
		World = context.contextSpatialGrid.world;
		IsDebug = context.isDebug;
		Members = context.contextSpatialGrid.members;
		MemberActive = context.contextSpatialGrid.memberActive;
		//DataReadIndex = context.contextSpatialGrid.dataReadIndex;
		
		SpatialGridSystems.InitializedSpatialGridSystems(context.contextSpatialGrid);
		SpatialGridSystems.BakeSpatialGridSystems();
	}
	
	void UpdateNavigationSystems()
	{
		SpatialGridSystems.UpdateSpatialGridSystem();
		
		//auto& members = (*Members)[*DataReadIndex];
		auto& members = *Members;
		
		int8 index = 0;
		for (int i = 0; i < *MemberActive; ++i)
		{
			if (index < 20 && members.CrowdsRequestNavigationPath[i])
			{
				RegeneratePath(members, i);
				index++;
				UE_LOG(LogTemp, Log, TEXT("NAVIGATION SYSTEMS:: REGENERATE"));
			}
			
			FVector location = FVector(members.CrowdsLocation[i].X, members.CrowdsLocation[i].Y, 0);
			int8 indexNavigationPath = members.CrowdsIndexNavigationPath[i];
			UE_LOG(LogTemp, Error, TEXT("CEK DISTANCE"));
			
			if (!members.CrowdsNavigationPath[i].LocationPaths->IsValidIndex(indexNavigationPath+1))
				continue;
			
			if (FVector::DistXY(location, members.CrowdsNavigationPath[i].LocationPaths[indexNavigationPath]) < 50)
				members.CrowdsIndexNavigationPath[i]++;
			
			float dist = FVector::DistSquaredXY(location, members.CrowdsNavigationPath[i].LocationPaths[members.CrowdsTotalNavigationPath[i]]);
			if (dist < 2500.0f) continue;
			
			if (IsDebug) ShowDebugText(members, i);
			
			if (members.CrowdsIndexNavigationPath[i] != members.CrowdsTotalNavigationPath[i]) continue;
			
			RegeneratePath(members, i);
		}
	}
	
	void EndPlayNavigationSystems()
	{
		World = nullptr;
	}
};
