#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/StateMachine/FluxPrimeBaseTransition.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "FluxPrimeCrowdsDistanceTransition.generated.h"

UENUM(BlueprintType)
enum class EFluxPrimeComparison : uint8
{
	Equal           UMETA(DisplayName = "=="),
	NotEqual        UMETA(DisplayName = "!="),
	GreaterThan     UMETA(DisplayName = ">"),
	GreaterEqual    UMETA(DisplayName = ">="),
	LessThan        UMETA(DisplayName = "<"),
	LessEqual       UMETA(DisplayName = "<="),
};

USTRUCT(BlueprintType)
struct FDistanceTransitionDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataTargetLocationTag;

	bool IsValid() const
	{
		return EntityDataLocationTag.IsValid() 
		   && EntityDataTargetLocationTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsDistanceTransition : public FFluxPrimeBaseTransition
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FDistanceTransitionDataTagConfig DataTagConfig;

	TArray<int16> EntityDataLocationID, EntityDataTargetLocationID;
	
	UPROPERTY(EditAnywhere)
	float Threshold = 0;
	
	UPROPERTY(EditAnywhere)
	EFluxPrimeComparison Condition = EFluxPrimeComparison::Equal;
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		for (auto& dataStore : dataStores)
		{
			EntityDataLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataLocationTag));
			EntityDataTargetLocationID.Add(dataStore.GetDataID(DataTagConfig.EntityDataTargetLocationTag));
		}
	}
	
	bool ExecuteTransition(int32 indexDataID, int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		FVector* entityDataLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataLocationID[indexDataID]));
		FVector* entityDataTargetLocation = reinterpret_cast<FVector*>(dataStore.GetRawBufferData(EntityDataTargetLocationID[indexDataID]));
		
		switch (Condition)
		{
		case EFluxPrimeComparison::Equal:
			return entityDataLocation[indexEntity].Equals(entityDataTargetLocation[indexEntity], Threshold);

		case EFluxPrimeComparison::NotEqual:
			return !entityDataLocation[indexEntity].Equals(entityDataTargetLocation[indexEntity], Threshold);

		case EFluxPrimeComparison::GreaterThan:
			return FVector::Dist2D(entityDataLocation[indexEntity], entityDataTargetLocation[indexEntity]) > Threshold;

		case EFluxPrimeComparison::GreaterEqual:
			return FVector::Dist2D(entityDataLocation[indexEntity], entityDataTargetLocation[indexEntity]) >= Threshold;

		case EFluxPrimeComparison::LessThan:
			return FVector::Dist2D(entityDataLocation[indexEntity], entityDataTargetLocation[indexEntity]) < Threshold;

		case EFluxPrimeComparison::LessEqual:
			return FVector::Dist2D(entityDataLocation[indexEntity], entityDataTargetLocation[indexEntity]) <= Threshold;

		default:
			return false;
		}
	}
};
