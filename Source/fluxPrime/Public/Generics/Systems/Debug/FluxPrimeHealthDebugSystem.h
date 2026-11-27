#pragma once
#include "Framework/Systems/Debug/FluxPrimeBaseDebugSystem.h"
#include "FluxPrimeHealthDebugSystem.generated.h"
USTRUCT(BlueprintType)
struct FLUXPRIME_API FFluxPrimeHealthDebugSystem : public FFluxPrimeBaseDebugSystem
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig Location;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig Health;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig ConditionAlive;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	bool bDrawBar = true;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	float BarWidth = 60;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	float BarHeight = 8;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	float BarZ = 560;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	float FontScale = 0.8f;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	FColor AliveColor = FColor::Green;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	FColor DeadColor = FColor::Red;
	
	void Initialized(const FFluxPrimeDebugInitContext& Context);
	void ExecuteDebug(const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) const;
};
