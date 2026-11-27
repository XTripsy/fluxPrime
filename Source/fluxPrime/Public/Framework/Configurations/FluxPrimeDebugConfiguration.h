#pragma once
#include "Engine/DataAsset.h"
#include "Framework/Systems/Debug/FluxPrimeBaseDebugSystem.h"
#include "StructUtils/InstancedStruct.h"
#include "FluxPrimeDebugConfiguration.generated.h"

UCLASS(BlueprintType)
class FLUXPRIME_API UFluxPrimeDebugConfiguration : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Budget")
	FFluxPrimeDebugLimits GlobalLimits;
	
	UPROPERTY(EditAnywhere, Category = "Candidate Rows")
	FFluxPrimeDataConfig CandidateLocation;
	
	UPROPERTY(EditAnywhere, Category = "Text Layout")
	float BaseTextHeightOffset = 180;
	
	UPROPERTY(EditAnywhere, Category = "Text Layout")
	float TextHeightStepPerSystem = 24;
	
	UPROPERTY(EditAnywhere, Category = "Debug Systems", meta = (BaseStruct = "/Script/fluxPrime.FluxPrimeBaseDebugSystem", ExcludeBaseStruct))
	TArray<FInstancedStruct> DebugSystems;
};
