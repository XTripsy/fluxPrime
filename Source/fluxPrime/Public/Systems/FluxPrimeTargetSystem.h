#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseSystem.h"
#include "Cores/FluxPrimeEvent.h"
#include "Cores/FluxPrimeStruct.h"
#include "FluxPrimeTargetSystem.generated.h"

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
	TArray<uint16>* targetCrowdsID = nullptr;
	TArray<int16>* idCrowds = nullptr;
	TArray<int8>* typeCrowds = nullptr;
	TArray<FFluxPrimeTargetCatalog>* crowdsTarget = nullptr;
	TArray<float>* crowdsAggroDistance = nullptr;
	uint16* memberActive = nullptr;
	FOnCrowdsManagerActionChange* onCrowdsManagerActionChange = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeTargetSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UWorld> World = nullptr;
	
	UPROPERTY()
	bool IsDebug = false;
	
	TArray<FVector>* LocationCrowds = nullptr;
	TArray<FVector>* LocationTargetCrowds = nullptr;
	TArray<uint16>* TargetCrowdsID = nullptr;
	TArray<int16>* IdCrowds = nullptr;
	TArray<int8>* TypeCrowds = nullptr;
	TArray<FFluxPrimeTargetCatalog>* CrowdsTarget = nullptr;
	uint16* MemberActive = nullptr;
	
	FOnCrowdsManagerActionChange* OnCrowdsManagerActionChange = nullptr;
	
	UPROPERTY()
	TArray<float> CrowdsAggroDistance;
	
private:
	void ShowDebug(FVector position, FColor color)
	{
		DrawDebugSphere(
			World,
			position,
			50.0f,
			8,
			color,
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
		check(context.targetCrowdsID);
		check(context.idCrowds);
		check(context.typeCrowds);
		check(context.crowdsTarget);
		check(context.memberActive);
		check(context.onCrowdsManagerActionChange);
		
		World = context.world;
		IsDebug = context.isDebug;
		LocationCrowds = context.locationCrowds;
		LocationTargetCrowds = context.locationTargetCrowds;
		TargetCrowdsID = context.targetCrowdsID;
		IdCrowds = context.idCrowds;
		TypeCrowds = context.typeCrowds;
		CrowdsTarget = context.crowdsTarget;
		CrowdsAggroDistance = *context.crowdsAggroDistance;
		MemberActive = context.memberActive;
		OnCrowdsManagerActionChange = context.onCrowdsManagerActionChange;
	}
	
	void UpdateTargetSystems()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Target_Systems);
		
		auto& locationCrowds = *LocationCrowds;
		auto& idCrowds = *IdCrowds;
		auto& typeCrowds = *TypeCrowds;
		auto& targetLocationCrowds = *LocationTargetCrowds;
		auto& targetCrowdsID = *TargetCrowdsID;
		auto& targetCrowds = *CrowdsTarget;
		
		TArray<FVector> targetLocations;
		TArray<uint16> targetPriority;
		targetLocations.SetNumUninitialized(targetCrowds.Num());
		targetPriority.SetNumUninitialized(targetCrowds.Num());

		for (int32 i = 0; i < targetCrowds.Num(); ++i)
		{
			if (!targetCrowds[i].CrowdsTarget.IsValid()) continue;

			targetLocations[i] = targetCrowds[i].CrowdsTarget->GetActorLocation();
			targetPriority[i] = targetCrowds[i].CrowdsTargetPriority;
		}
		
		float bestDistanceSquared = TNumericLimits<float>::Max();
		int32 bestPriority = TNumericLimits<int32>::Lowest();
		int16 bestTargetID = INDEX_NONE;
		
		for (int32 i = 0; i < *MemberActive; ++i)
		{
			bestDistanceSquared = TNumericLimits<float>::Max();
			bestPriority = TNumericLimits<int32>::Lowest();
			bestTargetID = INDEX_NONE;

			for (int j = 0; j < targetCrowds.Num(); ++j)
			{
				const float distanceSquared = FVector::DistSquaredXY(locationCrowds[i], targetLocations[j]);
				const float aggroDistanceSquared = FMath::Square(CrowdsAggroDistance[typeCrowds[i]]);
				
				if (distanceSquared > aggroDistanceSquared) continue;
				
				const int32 priority = targetPriority[j];
				
				if (priority > bestPriority)
				{
					bestPriority = priority;
					bestDistanceSquared = distanceSquared;
					bestTargetID = j;

					continue;
				}

				if (priority == bestPriority &&
					distanceSquared < bestDistanceSquared)
				{
					bestDistanceSquared = distanceSquared;
					bestTargetID = j;
				}
			}

			if (targetCrowdsID[i] != bestTargetID && bestTargetID != INDEX_NONE)
			{
				targetCrowdsID[i] = static_cast<uint16>(bestTargetID);
				
				FFluxPrimeChangeTargetActionPayload data;
				data.CrowdID = idCrowds[i];
				data.CrowdType = typeCrowds[i];
				data.TargetID = static_cast<uint16>(bestTargetID);
				data.NewTargetLocation = targetLocations[bestTargetID];
				FInstancedStruct payload = FInstancedStruct::Make(data);
				
				OnCrowdsManagerActionChange->ExecuteIfBound(payload);
			}
			
			if (!targetLocations.IsValidIndex(targetCrowdsID[i])) continue;
			
			targetLocationCrowds[i] = targetLocations[targetCrowdsID[i]];
		}
	}
};
