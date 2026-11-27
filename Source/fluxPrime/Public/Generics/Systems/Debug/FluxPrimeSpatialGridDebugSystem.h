#pragma once
#include "Framework/Systems/Debug/FluxPrimeBaseDebugSystem.h"
#include "FluxPrimeSpatialGridDebugSystem.generated.h"
USTRUCT(BlueprintType)
struct FLUXPRIME_API FFluxPrimeSpatialGridDebugSystem : public FFluxPrimeBaseDebugSystem
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig Location;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig CellId;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig Size;
	
	UPROPERTY(EditAnywhere, Category="Geometry")
	float CellSize = 100;
	
	UPROPERTY(EditAnywhere, Category="Geometry")
	FVector Origin = FVector(-10000,-10000,0);
	
	UPROPERTY(EditAnywhere, Category="Geometry")
	int32 CellWidth = 200;
	
	UPROPERTY(EditAnywhere, Category="Geometry")
	int32 CellHeight = 200;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	float BoxHeight = 50;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	float FontScale = .8f;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	FColor CellColor = FColor::Silver;
	
	mutable TMap<int32, int32> SampledOccupancy;
	mutable TMap<int32, uint64> FirstRows;
	
	void Initialized(const FFluxPrimeDebugInitContext& Context);
	void ExecuteDebug(const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) const;
};
