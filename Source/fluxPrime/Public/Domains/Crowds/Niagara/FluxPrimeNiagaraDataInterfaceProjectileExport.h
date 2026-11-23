#pragma once

#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "NiagaraDataInterface.h"
#include "FluxPrimeNiagaraDataInterfaceProjectileExport.generated.h"

/**
 * CPU-only export data interface for the batched projectile system.
 *
 * UNiagaraDataInterfaceExport can only hand back Position, Size and Velocity, which is not enough to
 * keep per-shot damage accurate once many shots share one system. This interface mirrors the engine
 * export path and adds Damage and ShotId, so a single batched system can report which shot every
 * particle belongs to.
 */
UCLASS(EditInlineNew, CollapseCategories, meta = (DisplayName = "Projectile Export"))
class FLUXPRIME_API UNiagaraDataInterfaceProjectileExport : public UNiagaraDataInterface
{
	GENERATED_BODY()

public:
	UNiagaraDataInterfaceProjectileExport(const FObjectInitializer& ObjectInitializer);
	virtual void PostInitProperties() override;

	/** User parameter holding the object that implements IFluxPrimeProjectileCallbackHandler. */
	UPROPERTY(EditAnywhere, Category = "Export")
	FNiagaraUserParameterBinding CallbackHandlerParameter;

	// UNiagaraDataInterface
	virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData,
		FVMExternalFunction& OutFunc) override;
	virtual bool InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual void DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	virtual int32 PerInstanceDataSize() const override;
	virtual bool PerInstanceTick(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	virtual bool PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	virtual bool Equals(const UNiagaraDataInterface* Other) const override;
	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override { return Target == ENiagaraSimTarget::CPUSim; }
	virtual bool HasPreSimulateTick() const override { return true; }
	virtual bool HasPostSimulateTick() const override { return true; }
	virtual bool PostSimulateCanOverlapFrames() const override { return false; }
	// UNiagaraDataInterface End

	void ExportProjectileData(FVectorVMExternalFunctionContext& Context);

protected:
#if WITH_EDITORONLY_DATA
	virtual void GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
#endif
	virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;
};
