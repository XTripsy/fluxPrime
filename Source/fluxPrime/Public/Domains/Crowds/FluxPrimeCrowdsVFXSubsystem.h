#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "FluxPrimeCrowdsVFXSubsystem.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UNiagaraDataChannelAsset;

USTRUCT()
struct FFluxPrimeCrowdsVFXListener
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraDataChannelAsset> Channel = nullptr;
	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> Component = nullptr;
	UPROPERTY(Transient)
	FName PositionVariable;
};

/** Cosmetic notify listeners only. One listener per global channel prevents duplicate consumption. */
UCLASS()
class FLUXPRIME_API UFluxPrimeCrowdsVFXSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	static bool IsConfigurationValid(UNiagaraSystem* System, UNiagaraDataChannelAsset* Channel, FName PositionVariable);
	bool Publish(UNiagaraSystem* System, UNiagaraDataChannelAsset* Channel, FName PositionVariable, const FVector& Position);
	UNiagaraComponent* FindListener(const UNiagaraDataChannelAsset* Channel) const;
	void ShutdownListeners();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	UPROPERTY(Transient)
	TArray<FFluxPrimeCrowdsVFXListener> Listeners;
	bool bShuttingDown = false;
};
