#pragma once
#include "Framework/Systems/Debug/FluxPrimeBaseDebugSystem.h"
#include "FluxPrimeAnimationDebugSystem.generated.h"
USTRUCT(BlueprintType)
struct FLUXPRIME_API FFluxPrimeAnimationDebugSystem : public FFluxPrimeBaseDebugSystem
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig Location;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig State;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig AnimState;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig StartTimeAnim;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig RequestAnimationNotify;
	
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig TargetLocation;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	bool bDrawTargetLine = false;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	float FontScale = 0.8f;
	
	UPROPERTY(EditAnywhere, Category="Presentation")
	FColor TargetLineColor = FColor::Blue;
	
	void Initialized(const FFluxPrimeDebugInitContext& Context);
	void ExecuteDebug(const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) const;
};
