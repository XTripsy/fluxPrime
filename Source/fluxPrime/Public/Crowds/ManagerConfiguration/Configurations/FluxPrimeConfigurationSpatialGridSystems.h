#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeBaseConfiguration.h"
#include "FluxPrimeConfigurationSpatialGridSystems.generated.h"

USTRUCT(BlueprintType)
struct FFluxPrimeConfigurationSpatialGridSystems : public FFluxPrimeBaseConfiguration
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Place the origin in left buttom of the world, because the system work from left to right(X) and go the top(Y)"))
	FVector Origin = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Cell size is size of every single cell"))
	float CellSize = 0.0f;
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Cell width is total cell from origin to right(X) of the world"))
	int32 CellWidth = 0;
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Cell height is totoal cell from origin to top(Y) of the world"))
	int32 CellHeight = 0;
	
	UPROPERTY(EditAnywhere, meta = (ToolTip = "Show Debug System"))
	bool IsDebug = false;
};