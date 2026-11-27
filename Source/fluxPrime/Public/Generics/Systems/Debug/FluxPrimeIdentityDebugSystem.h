#pragma once
#include "Framework/Systems/Debug/FluxPrimeBaseDebugSystem.h"
#include "FluxPrimeIdentityDebugSystem.generated.h"
USTRUCT(BlueprintType)
struct FLUXPRIME_API FFluxPrimeIdentityDebugSystem : public FFluxPrimeBaseDebugSystem
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig Location;
	UPROPERTY(EditAnywhere, Category="Data")
	FFluxPrimeDataConfig StoredId;
	UPROPERTY(EditAnywhere, Category="Presentation")
	float FontScale = 0.8f;
	void Initialized(const FFluxPrimeDebugInitContext& Context);
	void ExecuteDebug(const FFluxPrimeDebugExecuteContext& Context, FFluxPrimeDebugFrame& Frame) const;
};
