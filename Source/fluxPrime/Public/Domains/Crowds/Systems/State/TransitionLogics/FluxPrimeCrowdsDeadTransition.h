#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/StateMachine/FluxPrimeBaseTransition.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "FluxPrimeCrowdsDeadTransition.generated.h"

USTRUCT(BlueprintType)
struct FDeadTransitionDataTagConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag Config")
	FGameplayTag EntityDataHealthTag;
	
	bool IsValid() const
	{
		return EntityDataHealthTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FFluxPrimeCrowdsDeadTransition : public FFluxPrimeBaseTransition
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere)
	FDeadTransitionDataTagConfig DataTagConfig;
	
	TArray<int16> EntityDataHealthID;
	
public:
	void Initialized(TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		for (auto& dataStore : dataStores)
		{
			EntityDataHealthID.Add(dataStore.GetDataID(DataTagConfig.EntityDataHealthTag));
		}
	}
	
	bool ExecuteTransition(int32 indexDataID, int32 indexEntity, FFluxPrimeArchetypeDataStore& dataStore)
	{
		FFluxPrimeWrapperInt16* entityDataHealth = reinterpret_cast<FFluxPrimeWrapperInt16*>(dataStore.GetRawBufferData(EntityDataHealthID[indexDataID]));
		
		if (entityDataHealth[indexEntity].ValueInt16 > 0) return false;
		
		UE_LOG(LogTemp, Log, TEXT("TRANSITION:: DEAD LOGIC TRANSITION"));
		return true;
	}
};
