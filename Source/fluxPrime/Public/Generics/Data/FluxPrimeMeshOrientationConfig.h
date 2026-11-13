#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeMeshOrientationConfig.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeMeshOrientationConfig
{
	GENERATED_BODY()

	// Degrees added to world travel yaw to obtain mesh yaw. Abilities use the inverse.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Orientation")
	float ForwardYawOffsetDegrees = -85.0f;

	float ToMeshYaw(float TravelYaw) const { return TravelYaw + ForwardYawOffsetDegrees; }
	float ToTravelYaw(float MeshYaw) const { return MeshYaw - ForwardYawOffsetDegrees; }

	static const FFluxPrimeMeshOrientationConfig& Default()
	{
		static const FFluxPrimeMeshOrientationConfig Config;
		return Config;
	}
};
