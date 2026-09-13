#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseIdentity.h"
#include "FluxPrimeDataStoreIdentity.h"
#include "FluxPrimeIdentityData.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeArchetypeEntityCatalog
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UFluxPrimeBaseIdentity> EntityIdentity;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UFluxPrimeDataStoreIdentity> DataStoreIdentity;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	int32 TotalEntityOnPool;
};

USTRUCT(BlueprintType)
struct FFluxPrimeEntityLookup
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	int32 EntityID;
	
	UPROPERTY(EditAnywhere)
	int8 EntityType;
	
	bool operator==(const FFluxPrimeEntityLookup& Other) const
	{
		return EntityID == Other.EntityID && EntityType == Other.EntityType;
	}
};