#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeSpatialGridSystems.h"
#include "FluxPrimeDamageSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeDamageSystems
{
	GENERATED_BODY()
	
public:
	void TakeDamage(TObjectPtr<UWorld> world, TSharedPtr<FFluxPrimeSpatialGridSystems> spatialGridSystems, FFluxPrimeCrowds& members, int32 id, int32 memberActive)
	{
		for (int i = 0; i < memberActive; ++i)
		{
			if (members.CrowdsID[i] != id) continue;
			
			FVector location = members.CrowdsLocation[i];
			int32 cellID = spatialGridSystems->GetSpatialGridSystemsCellID(location);
			
			if (members.CrowdsCellID[i] != cellID) continue;
			
			members.CrowdsStartTimeAnimationFrame[i] = world->GetRealTimeSeconds();
			members.CrowdsAnimationIndex[i] += 2;
		}
	}
};
