#pragma once
#include "Framework/Systems/Debug/FluxPrimeBaseDebugSystem.h"
#include "FluxPrimeNavigationDebugSystem.generated.h"
USTRUCT(BlueprintType)
struct FLUXPRIME_API FFluxPrimeNavigationDebugSystem : public FFluxPrimeBaseDebugSystem
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig CurrentLocation;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig TargetLocation;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig CurrentWaypoint;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig Waypoints;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig CountWaypoints;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig CountCorridor;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig NeedReplan;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig WaitTime;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	FColor PathColor = FColor::Cyan;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	FColor TargetColor = FColor::Green;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	FColor ReplanColor = FColor::Red;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	int32 MaxWaypointsDrawn = 8;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	float FontScale = .8f;
	
	void Initialized(const FFluxPrimeDebugInitContext& Context);
	void ExecuteDebug(const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) const;
};
