#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsData.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "GameplayTagContainer.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeCrowdsTargetSystem.generated.h"

USTRUCT(BlueprintType)
struct FTargetSystemDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataTargetLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataTargetIdTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataIdTag;

	bool IsValid() const
	{
		return EntityDataLocationTag.IsValid() 
			&& EntityDataTargetLocationTag.IsValid() 
			&& EntityDataTargetIdTag.IsValid() 
			&& EntityDataIdTag.IsValid();
	}
};

USTRUCT()
struct FFluxPrimeCrowdsTargetSystemsContext
{
	GENERATED_BODY()
	
	TArray<FFluxPrimeTargetCatalog>* targetCatalogs = nullptr;
	TArray<float>* aggroDistance = nullptr;
	TArray<FFluxPrimeArchetypeDataStore>* dataStores = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsTargetSystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
private:
	TArray<FFluxPrimeTargetCatalog>* TargetCatalogs = nullptr;
	
	UPROPERTY()
	TArray<float> AggroDistance;
	
	UPROPERTY(EditAnywhere)
	FTargetSystemDataTagConfig DataTagConfig;
	
	float CurrentTime = 0;
	
	UPROPERTY(EditAnywhere)
	float RandomCircleRadius = 150.0f;
	
	UPROPERTY(EditAnywhere)
	float PriorityBonusDistancePerPoint = 500.0f;
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "1s is equal 1Hz, so its mean 0.1s is equal 10Hz"))
	float CycleFrequencyBuffer = 0.1f;
	
	UPROPERTY(EditAnywhere)
	int32 MaxAttackersPerTarget = 25;

	UPROPERTY(EditAnywhere)
	float CrowdLoadPenaltyDistance = 250.0f;

	UPROPERTY(EditAnywhere)
	float TargetStickinessDistance = 300.0f;
	
	TArray<int16> EntityDataLocationID, EntityDataTargetLocationID, EntityDataTargetIdID, EntityDataIdID;
	
public:
	void Initialized(FFluxPrimeCrowdsTargetSystemsContext context)
	{
		check(context.targetCatalogs);
		check(context.dataStores);
		
		TargetCatalogs = context.targetCatalogs;
		AggroDistance = *context.aggroDistance;
		
		for (auto& dataStore : *context.dataStores)
		{
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataTargetLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataTargetLocationTag));
			EntityDataTargetIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataTargetIdTag));
			EntityDataIdID.Add(dataStore.GetDataID(DataTagConfig.EntityDataIdTag));
		}
	}
	
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
	    TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Target_Systems);
	    
	    check(TargetCatalogs);
	    if (TargetCatalogs->IsEmpty()) return;
	    
	    CurrentTime += deltaTime;
	    
	    if (CurrentTime < CycleFrequencyBuffer) return;
	    CurrentTime = 0.0f;
	    
	    auto& targetCatalogs = *TargetCatalogs;
	    const int32 numTargets = targetCatalogs.Num();

	    TArray<FVector> targetLocations;
	    TArray<uint16> targetPriority;
	    TArray<int32> targetAttackerCounts;
	    
	    targetLocations.SetNumUninitialized(numTargets);
	    targetPriority.SetNumUninitialized(numTargets);
	    targetAttackerCounts.Init(0, numTargets);

	    for (int32 i = 0; i < numTargets; ++i)
	    {
	       if (!targetCatalogs[i].CrowdsTarget.IsValid()) continue;

	       targetLocations[i] = targetCatalogs[i].CrowdsTarget->GetActorLocation();
	       targetPriority[i] = targetCatalogs[i].CrowdsTargetPriority;
	    }

	    const float crowdPenaltyPerAttackerSq = FMath::Square(CrowdLoadPenaltyDistance);
	    const float stickinessBonusSq = FMath::Square(TargetStickinessDistance);
	    
	    int16 indexDataID = 0;
	    for (auto& dataStore : dataStores)
	    {
	       FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
	       FVector* entityDataTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]));
	       FFluxPrimeWrapperInt16* entityDataTargetID = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataTargetIdID[indexDataID]));
	       
	       check(entityDataLocation);
	       check(entityDataTargetLocation);
	       check(entityDataTargetID);
	       
	       for (int32 i = 0; i < dataStore.GetActiveEntityCount(); ++i)
	       {
	          float bestEffectiveDistanceSquared = TNumericLimits<float>::Max();
	          int16 bestTargetID = INDEX_NONE;

	          const int16 currentTargetID = entityDataTargetID[i].ValueInt16;

	          for (int32 j = 0; j < numTargets; ++j)
	          {
	             if (!targetCatalogs[j].CrowdsTarget.IsValid()) continue;

	             if (MaxAttackersPerTarget > 0 && targetAttackerCounts[j] >= MaxAttackersPerTarget) continue;

	             const float rawDistanceSquared = FVector::DistSquaredXY(entityDataLocation[i], targetLocations[j]);
	             const float loadPenalty = targetAttackerCounts[j] * crowdPenaltyPerAttackerSq;
	             const float stickinessBonus = (currentTargetID == j) ? stickinessBonusSq : 0.0f;

	             const float priorityDistanceBonus = targetPriority[j] * PriorityBonusDistancePerPoint;
	             const float priorityBonusSq = FMath::Square(priorityDistanceBonus);

	             const float effectiveDistanceSquared = rawDistanceSquared + loadPenalty - stickinessBonus - priorityBonusSq;
	             
	             if (effectiveDistanceSquared < bestEffectiveDistanceSquared)
	             {
	                bestEffectiveDistanceSquared = effectiveDistanceSquared;
	                bestTargetID = j;
	             }
	          }

	          if (bestTargetID != INDEX_NONE)
	          {
	             targetAttackerCounts[bestTargetID]++;
	             entityDataTargetID[i].ValueInt16 = bestTargetID;

	             const FVector2D randomCircle = FMath::RandPointInCircle(RandomCircleRadius);
	             entityDataTargetLocation[i] = targetLocations[bestTargetID] + FVector(randomCircle.X, randomCircle.Y, 0.0f);
	          }
	          else
	          {
	             entityDataTargetID[i].ValueInt16 = INDEX_NONE;
	             entityDataTargetLocation[i] = entityDataLocation[i];
	          }
	       }
	       
	       indexDataID++;
	    }
	}
};
