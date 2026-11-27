#pragma once
#include "Framework/Systems/Debug/FluxPrimeBaseDebugSystem.h"
#include "FluxPrimeBoidsDebugSystem.generated.h"
USTRUCT(BlueprintType)
struct FLUXPRIME_API FFluxPrimeBoidsDebugSystem : public FFluxPrimeBaseDebugSystem
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig Location;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig Size;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	float DiameterScale = 1.2f;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	float FontScale = 0.8f;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	FColor CircleColor = FColor::Orange;
	
	void Initialized(const FFluxPrimeDebugInitContext& Context);
	void ExecuteDebug(const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) const;
};
