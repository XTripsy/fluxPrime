#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystems.h"
#include "FluxPrimeSpatialGridSystems.h"
#include "Cores/FluxPrimeStruct.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeNavigationSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeNavigationSystemsContext
{
	GENERATED_BODY()
	
	UPROPERTY()
	bool isDebug = false;
	
	TArray<FFluxPrimeCrowdsPath>* navigationPathCrowds = nullptr;
	TArray<FVector>* locationCrowds = nullptr;
	TArray<FVector>* locationCurrentTargetCrowds = nullptr;
	TArray<FVector>* locationTargetCrowds = nullptr;
	TArray<int32>* crowdsCellID = nullptr;
	TArray<bool>* requestNavigationPathCrowds = nullptr;
	TArray<int8>* indexNavigationPathCrowds = nullptr;
	TArray<int8>* totalNavigationPathCrowds = nullptr;
	
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
	
	TArray<FFluxPrimeCrowdsPath>* NavigationPathCrowds = nullptr;
	TArray<FVector>* LocationCrowds = nullptr;
	TArray<FVector>* LocationCurrentTargetCrowds = nullptr;
	TArray<FVector>* LocationTargetCrowds = nullptr;
	TArray<int32>* CrowdsCellID = nullptr;
	TArray<bool>* RequestNavigationPathCrowds = nullptr;
	TArray<int8>* IndexNavigationPathCrowds = nullptr;
	TArray<int8>* TotalNavigationPathCrowds = nullptr;
	uint16* MemberActive = nullptr;
	
	UPROPERTY()
	TArray<FFluxPrimeCrowdsPath> NavigationPaths;
	
	UPROPERTY()
	TArray<int16> TotalNavigationPaths;
	
	UPROPERTY()
	TArray<int32> TargetCellIdPaths;
	
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
	
	void ShowDebugText(TArray<FVector>& locationCrowds, TArray<int8>& indexCrowds, 
		TArray<int8>& totalCrowds, int32 indexMembers)
	{
		FVector textLocation = locationCrowds[indexMembers] + (FVector::UpVector * FluxConfig::DebugLocationNavigation);
		FString debugData = FString::Printf(TEXT("Navigation Index Target: %d \n Navigation Total Path: %d"), indexCrowds[indexMembers], totalCrowds[indexMembers]);
		
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
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_Calculate_Path_Systems);
		
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
	
	bool UpdatePathCache(const int32 cellId, const FVector locationCrowds, const FVector targetCrowds)
	{
		TArray<FVector> path;
		if (!CalculatePath(locationCrowds, targetCrowds, path)) return false;
		
		UE_LOG(LogTemp, Log, TEXT("NAVIGATION SYSTEMS:: REGENERATE SUCCESS"));
		int16 total = FMath::Min(path.Num() - 1, FluxConfig::NavigationArrayCount);
		TotalNavigationPaths[cellId] = total;
		
		for (int8 j = 0; j < total; ++j)
		{
			path[j+1].Z = 0;
			NavigationPaths[cellId].LocationPaths[j] = path[j+1];
		}
		
		return true;
	}
	
	void UpdatePathCrowds(const int32 cellId, bool& requestCrowds, int8& indexCrowds,
		FVector& currentTargetCrowds, int8& totalCrowds, FFluxPrimeCrowdsPath& pathCrowds)
	{
		for (int8 j = 0; j < FluxConfig::NavigationArrayCount; ++j)
		{
			pathCrowds.LocationPaths[j] = NavigationPaths[cellId].LocationPaths[j];
		}
		
		indexCrowds = 0;
		totalCrowds = TotalNavigationPaths[cellId];
		requestCrowds = false;
		currentTargetCrowds = pathCrowds.LocationPaths[indexCrowds];
	}
	
	void UpdatePaths(const int32& cellID, FVector& currentTargetCrowds,const FVector& targetCrowds, const FVector& locationCrowds,
		FFluxPrimeCrowdsPath& navigationPathCrowds, bool& requestPathCrowds, int8& indexPathCrowds, int8& totalPathCrowds)
	{
		int32 memberCellId = cellID;
		int32 memberTargetCellID = SpatialGridSystems.GetSpatialGridSystemsCellID(targetCrowds);
		
		// masih ada bug, harus di clear
		if (TargetCellIdPaths[memberCellId] != memberTargetCellID)
		{
			if (!UpdatePathCache(memberCellId, locationCrowds, targetCrowds)) return;
			UpdatePathCrowds(memberCellId, requestPathCrowds, indexPathCrowds, currentTargetCrowds, totalPathCrowds, navigationPathCrowds);
			TargetCellIdPaths[memberCellId] = memberTargetCellID;
		}
		else
		{
			UpdatePathCrowds(memberCellId, requestPathCrowds, indexPathCrowds, currentTargetCrowds, totalPathCrowds, navigationPathCrowds);
		}
	}
	
public:
	void InitializedNavigationSystems(FFluxPrimeNavigationSystemsContext context)
	{
		check(context.contextSpatialGrid.memberActive);
		check(context.locationCrowds);
		check(context.navigationPathCrowds);
		check(context.indexNavigationPathCrowds);
		check(context.totalNavigationPathCrowds);
		check(context.locationCurrentTargetCrowds);
		check(context.requestNavigationPathCrowds);
		check(context.locationTargetCrowds);
		check(context.crowdsCellID);
		
		World = context.contextSpatialGrid.world;
		IsDebug = context.isDebug;
		LocationCrowds = context.locationCrowds;
		CrowdsCellID = context.crowdsCellID;
		RequestNavigationPathCrowds = context.requestNavigationPathCrowds;
		NavigationPathCrowds = context.navigationPathCrowds;
		IndexNavigationPathCrowds = context.indexNavigationPathCrowds;
		TotalNavigationPathCrowds = context.totalNavigationPathCrowds;
		LocationCurrentTargetCrowds = context.locationCurrentTargetCrowds;
		LocationTargetCrowds = context.locationTargetCrowds;
		MemberActive = context.contextSpatialGrid.memberActive;
		
		SpatialGridSystems.InitializedSpatialGridSystems(context.contextSpatialGrid);
		SpatialGridSystems.BakeSpatialGridSystems();
		
		int32 totalCells = SpatialGridSystems.GetTotalCells();
		NavigationPaths.SetNumZeroed(totalCells);
		TotalNavigationPaths.Init(0, totalCells);
		TargetCellIdPaths.Init(INDEX_NONE, totalCells);
	}
	
	void UpdateNavigationSystems()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Navigation_Systems);
		
		SpatialGridSystems.UpdateSpatialGridSystem();
		
		auto& requestPathCrowds = *RequestNavigationPathCrowds;
		auto& locationCrowds = *LocationCrowds;
		auto& cellId = *CrowdsCellID;
		auto& indexPathCrowds = *IndexNavigationPathCrowds;
		auto& totalPathCrowds = *TotalNavigationPathCrowds;
		auto& navigationPathCrowds = *NavigationPathCrowds;
		auto& currentTargetCrowds = *LocationCurrentTargetCrowds;
		auto& targetCrowds = *LocationTargetCrowds;

		int8 index = 0;
		for (int i = 0; i < *MemberActive; ++i)
		{
			if (index < 20 && requestPathCrowds[i])
			{
				UpdatePaths(cellId[i], currentTargetCrowds[i], targetCrowds[i],
					locationCrowds[i], navigationPathCrowds[i], requestPathCrowds[i], 
					indexPathCrowds[i], totalPathCrowds[i]);
				index++;
			}
			
			FVector location = FVector(locationCrowds[i].X, locationCrowds[i].Y, 0);
			int8 indexNavigationPath = indexPathCrowds[i];
			
			if (!navigationPathCrowds[i].LocationPaths->IsValidIndex(indexNavigationPath+1)) continue;
			
			if (FVector::DistXY(location, currentTargetCrowds[i]) < 50)
			{
				indexPathCrowds[i]++;
				currentTargetCrowds[i] = navigationPathCrowds[i].LocationPaths[indexPathCrowds[i]]; 
			}
			
			float dist = FVector::DistSquaredXY(location, navigationPathCrowds[i].LocationPaths[totalPathCrowds[i]]);
			if (dist < 2500.0f) continue;
			if (indexPathCrowds[i] != totalPathCrowds[i]) continue;
			
			UpdatePaths(cellId[i], currentTargetCrowds[i], targetCrowds[i],
					locationCrowds[i], navigationPathCrowds[i], requestPathCrowds[i], 
					indexPathCrowds[i], totalPathCrowds[i]);
		}
	}
	
	void EndPlayNavigationSystems()
	{
		World = nullptr;
	}
};
