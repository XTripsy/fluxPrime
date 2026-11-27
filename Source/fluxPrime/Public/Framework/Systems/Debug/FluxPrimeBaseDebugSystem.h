#pragma once
#include "CoreMinimal.h"
#include "Framework/Core/FluxPrimeDebugReadView.h"
#include "Framework/Data/FluxPrimeDebugData.h"
#include "FluxPrimeBaseDebugSystem.generated.h"

struct FFluxPrimeDebugInitContext
{
	const FFluxPrimeDebugReadView* View = nullptr;
	TConstArrayView<FFluxPrimeDebugArchetypeMeta> Archetypes;
};

struct FFluxPrimeDebugExecuteContext
{
	const FFluxPrimeDebugReadView* View = nullptr;
	TConstArrayView<FFluxPrimeDebugArchetypeMeta> Archetypes;
	TConstArrayView<FFluxPrimeDebugRowRef> Candidates;
	FVector CameraLocation = FVector::ZeroVector;
	TConstArrayView<FFluxPrimeDebugRowRef> AllCandidates;
};

USTRUCT(BlueprintType)
struct FLUXPRIME_API FFluxPrimeBaseDebugSystem
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bEnabled = true;
	
	UPROPERTY(EditAnywhere, Category = "Debug")
	FColor DebugColor = FColor::White;
	
	UPROPERTY(Transient)
	int32 VisualOrderIndex = 0;
	
	UPROPERTY(Transient)
	bool bIsDataValid = false;
	
	void Initialized(const FFluxPrimeDebugInitContext&) {}
	void ExecuteDebug(const FFluxPrimeDebugExecuteContext&, FFluxPrimeDebugFrame&) const {}
};
