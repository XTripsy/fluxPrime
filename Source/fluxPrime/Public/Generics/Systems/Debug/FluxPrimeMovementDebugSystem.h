#pragma once
#include "Framework/Systems/Debug/FluxPrimeBaseDebugSystem.h"
#include "FluxPrimeMovementDebugSystem.generated.h"
USTRUCT(BlueprintType)
struct FLUXPRIME_API FFluxPrimeMovementDebugSystem : public FFluxPrimeBaseDebugSystem
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig Location;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig Velocity;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig Waypoint;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig MaxSpeed;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	float FontScale = 0.8f;
	
	void Initialized(const FFluxPrimeDebugInitContext& Context);
	void ExecuteDebug(const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) const;
};
