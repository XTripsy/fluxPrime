#include "Domains/Crowds/Niagara/FluxPrimeCrowdsNiagaraDataInterfaceProjectileExport.h"

#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileCallbackHandler.h"
#include "Domains/Crowds/Niagara/FluxPrimeCrowdsProjectileDiagnostics.h"
#include "fluxPrime.h"
#include "Misc/LargeWorldRenderPosition.h"
#include "NiagaraSystem.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraTypes.h"
#include "NiagaraWorldManager.h"
#include "NiagaraEmitterInstance.h"
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_ATOMIC_INT_COUNTER(FluxProjectileExportArrayGrowth, TEXT("FluxPrime/Projectile/PluginExportArrayGrowthEvents"));
TRACE_DECLARE_ATOMIC_INT_COUNTER(FluxProjectileExportArrayBytes, TEXT("FluxPrime/Projectile/PluginExportArrayLastAllocatedBytes"));
TRACE_DECLARE_ATOMIC_INT_COUNTER(FluxProjectileExportQueueNodes, TEXT("FluxPrime/Projectile/PluginExportQueueNodeAllocations"));
TRACE_DECLARE_ATOMIC_INT_COUNTER(FluxProjectileActiveParticles, TEXT("FluxPrime/Projectile/LastExportSystemReportedActiveParticles"));

namespace FluxPrimeProjectileExportLocal
{
	static const FName ExportFunctionName(TEXT("ExportProjectileParticles"));

	struct FInstanceData
	{
		TWeakObjectPtr<UObject> CallbackHandler;

		/** Written on the simulation thread, drained on the game thread, so it must be lock free. */
		TQueue<FFluxPrimeProjectileParticleData, EQueueMode::Mpsc> GatheredData;

		FNiagaraParameterDirectBinding<UObject*> UserParamBinding;
		bool bCanExport = false;
	};

	void DispatchToGameThread(TWeakObjectPtr<UObject> WeakCallbackHandler,
		TArray<FFluxPrimeProjectileParticleData> Particles, TWeakObjectPtr<UNiagaraSystem> WeakSystem,
		FVector3f SystemTileOffset)
	{
		FNiagaraWorldManager::EnqueueGlobalDeferredCallback(
			[WeakCallbackHandler, Particles = MoveTemp(Particles), WeakSystem, SystemTileOffset]()
			{
				TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrimeProjectileExportGTCallback);

				UObject* CallbackHandler = WeakCallbackHandler.Get();
				if (CallbackHandler == nullptr)
				{
					return;
				}

				IFluxPrimeCrowdsProjectileCallbackHandler::Execute_ReceiveProjectileParticles(
					CallbackHandler, Particles, WeakSystem.Get(),
					FVector(SystemTileOffset) * FLargeWorldRenderScalar::GetTileSize());
			});
	}
}

UNiagaraDataInterfaceProjectileExport::UNiagaraDataInterfaceProjectileExport(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CallbackHandlerParameter.Parameter.SetType(FNiagaraTypeDefinition(UObject::StaticClass()));
	CallbackHandlerParameter.Parameter.SetName(TEXT("User.ProjectileCallback"));
}

void UNiagaraDataInterfaceProjectileExport::PostInitProperties()
{
	Super::PostInitProperties();
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()),
			ENiagaraTypeRegistryFlags::AllowAnyVariable | ENiagaraTypeRegistryFlags::AllowParameter);
	}
}

bool UNiagaraDataInterfaceProjectileExport::InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	new (PerInstanceData) FluxPrimeProjectileExportLocal::FInstanceData;
	return true;
}

void UNiagaraDataInterfaceProjectileExport::DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	static_cast<FluxPrimeProjectileExportLocal::FInstanceData*>(PerInstanceData)->~FInstanceData();
}

int32 UNiagaraDataInterfaceProjectileExport::PerInstanceDataSize() const
{
	return sizeof(FluxPrimeProjectileExportLocal::FInstanceData);
}

bool UNiagaraDataInterfaceProjectileExport::PerInstanceTick(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance,
	float DeltaSeconds)
{
	using FInstanceData = FluxPrimeProjectileExportLocal::FInstanceData;

	FInstanceData* InstanceData = static_cast<FInstanceData*>(PerInstanceData);
	if (InstanceData == nullptr || SystemInstance == nullptr)
	{
		return true;
	}

	if (CallbackHandlerParameter.Parameter.IsValid())
	{
		InstanceData->CallbackHandler = InstanceData->UserParamBinding.Init(
			SystemInstance->GetInstanceParameters(), CallbackHandlerParameter.Parameter);
	}
	else
	{
		InstanceData->CallbackHandler.Reset();
	}

	UObject* Handler = InstanceData->CallbackHandler.Get();
	InstanceData->bCanExport = InstanceData->UserParamBinding.BoundVariable.IsValid() && Handler
		&& Handler->GetClass()->ImplementsInterface(UFluxPrimeCrowdsProjectileCallbackHandler::StaticClass());
	return false;
}

bool UNiagaraDataInterfaceProjectileExport::PerInstanceTickPostSimulate(void* PerInstanceData,
	FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	using FInstanceData = FluxPrimeProjectileExportLocal::FInstanceData;

	FInstanceData* InstanceData = static_cast<FInstanceData*>(PerInstanceData);
	if (SystemInstance)
	{
		int64 ActiveParticles = 0;
		for (const FNiagaraEmitterInstanceRef& Emitter : SystemInstance->GetEmitters())
		{
			ActiveParticles += Emitter->GetNumParticles();
		}
		TRACE_COUNTER_SET(FluxProjectileActiveParticles, ActiveParticles);
	}
	if (InstanceData == nullptr || InstanceData->GatheredData.IsEmpty())
	{
		return false;
	}

	TArray<FFluxPrimeProjectileParticleData> Particles;
	FFluxPrimeProjectileParticleData Particle;
	while (InstanceData->GatheredData.Dequeue(Particle))
	{
		const int32 PreviousCapacity = Particles.Max();
		Particles.Add(Particle);
		if (Particles.Max() != PreviousCapacity)
		{
			TRACE_COUNTER_INCREMENT(FluxProjectileExportArrayGrowth);
		}
	}

	TRACE_COUNTER_SET(FluxProjectileExportArrayBytes, Particles.GetAllocatedSize());
	TRACE_COUNTER_ADD(FluxProjectileExportQueueNodes, Particles.Num());
	UObject* CallbackHandler = InstanceData->CallbackHandler.Get();
	if (CallbackHandler == nullptr || SystemInstance == nullptr)
	{
		return false;
	}

	if (!CallbackHandler->GetClass()->ImplementsInterface(UFluxPrimeCrowdsProjectileCallbackHandler::StaticClass()))
	{
		return false;
	}

	FluxPrimeProjectileExportLocal::DispatchToGameThread(InstanceData->CallbackHandler, MoveTemp(Particles),
		SystemInstance->GetSystem(), SystemInstance->GetLWCTile());

	return false;
}

bool UNiagaraDataInterfaceProjectileExport::Equals(const UNiagaraDataInterface* Other) const
{
	if (!Super::Equals(Other))
	{
		return false;
	}

	const UNiagaraDataInterfaceProjectileExport* OtherTyped = CastChecked<const UNiagaraDataInterfaceProjectileExport>(Other);
	return OtherTyped->CallbackHandlerParameter == CallbackHandlerParameter;
}

bool UNiagaraDataInterfaceProjectileExport::CopyToInternal(UNiagaraDataInterface* Destination) const
{
	if (!Super::CopyToInternal(Destination))
	{
		return false;
	}

	CastChecked<UNiagaraDataInterfaceProjectileExport>(Destination)->CallbackHandlerParameter = CallbackHandlerParameter;
	return true;
}

#if WITH_EDITORONLY_DATA
void UNiagaraDataInterfaceProjectileExport::GetFunctionsInternal(TArray<FNiagaraFunctionSignature>& OutFunctions) const
{
	FNiagaraFunctionSignature Signature;
	Signature.Name = FluxPrimeProjectileExportLocal::ExportFunctionName;
	Signature.Description = NSLOCTEXT("FluxPrime", "ExportProjectileParticlesDescription",
		"Stores the particle position, velocity, size, damage and shot id so they can be handed to the callback handler after the simulation has ticked.");
	Signature.bMemberFunction = true;
	Signature.bRequiresContext = false;
	Signature.bSupportsGPU = false;
	Signature.bRequiresExecPin = true;
	Signature.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Export interface")));
	Signature.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Store Data")));
	Signature.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Position")));
	Signature.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Velocity")));
	Signature.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Size")));
	Signature.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Damage")));
	Signature.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("ShotId")));
	Signature.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("WasExported")));
	OutFunctions.Add(Signature);

	FNiagaraFunctionSignature Counts;
	Counts.Name = TEXT("ReportProjectileCounts");
	Counts.Description = NSLOCTEXT("FluxPrime", "ReportProjectileCountsDescription",
		"CPU diagnostics only. Report actual successful reads, ball spawns and muzzle burst events at their graph execution sites. Never pass publication counts or call every particle update.");
	Counts.bMemberFunction = true;
	Counts.bRequiresContext = false;
	Counts.bSupportsGPU = false;
	Counts.bRequiresExecPin = true;
	Counts.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Export interface")));
	Counts.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("ConsumedRecords")));
	Counts.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("BallSpawns")));
	Counts.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("MuzzleBursts")));
	OutFunctions.Add(Counts);
}
#endif

DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceProjectileExport, ExportProjectileData);
DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceProjectileExport, ReportProjectileCounts);

void UNiagaraDataInterfaceProjectileExport::ReportProjectileCounts(FVectorVMExternalFunctionContext& Context)
{
	VectorVM::FUserPtrHandler<FluxPrimeProjectileExportLocal::FInstanceData> InstanceData(Context);
	FNDIInputParam<int32> Consumed(Context), Balls(Context), Muzzles(Context);
	uint64 ConsumedTotal = 0, BallTotal = 0, MuzzleTotal = 0;
	for (int32 Index = 0; Index < Context.GetNumInstances(); ++Index)
	{
		ConsumedTotal += FMath::Max(0, Consumed.GetAndAdvance());
		BallTotal += FMath::Max(0, Balls.GetAndAdvance());
		MuzzleTotal += FMath::Max(0, Muzzles.GetAndAdvance());
	}
	FFluxPrimeCrowdsProjectileDiagnostics::RecordGraphCounts(ConsumedTotal, BallTotal, MuzzleTotal);
}

void UNiagaraDataInterfaceProjectileExport::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo,
	void* InstanceData, FVMExternalFunction& OutFunc)
{
	if (BindingInfo.Name == TEXT("ReportProjectileCounts"))
	{
		NDI_FUNC_BINDER(UNiagaraDataInterfaceProjectileExport, ReportProjectileCounts)::Bind(this, OutFunc);
		return;
	}
	if (BindingInfo.Name == FluxPrimeProjectileExportLocal::ExportFunctionName)
	{
		NDI_FUNC_BINDER(UNiagaraDataInterfaceProjectileExport, ExportProjectileData)::Bind(this, OutFunc);
		return;
	}

	UE_LOG(LogFluxPrime, Warning, TEXT("Could not find data interface external function in %s. Expected Name: %s  Actual Name: %s"),
		*GetPathNameSafe(this), *FluxPrimeProjectileExportLocal::ExportFunctionName.ToString(), *BindingInfo.Name.ToString());
}

void UNiagaraDataInterfaceProjectileExport::ExportProjectileData(FVectorVMExternalFunctionContext& Context)
{
	using FInstanceData = FluxPrimeProjectileExportLocal::FInstanceData;

	VectorVM::FUserPtrHandler<FInstanceData> InstanceData(Context);

	FNDIInputParam<FNiagaraBool> StoreDataParam(Context);
	FNDIInputParam<FVector3f> PositionParam(Context);
	FNDIInputParam<FVector3f> VelocityParam(Context);
	FNDIInputParam<float> SizeParam(Context);
	FNDIInputParam<float> DamageParam(Context);
	FNDIInputParam<int32> ShotIdParam(Context);

	FNDIOutputParam<FNiagaraBool> OutWasExported(Context);

	checkfSlow(InstanceData.Get(), TEXT("Projectile export data interface has invalid instance data. %s"), *GetPathName());

	for (int32 Index = 0; Index < Context.GetNumInstances(); ++Index)
	{
		const bool bStore = StoreDataParam.GetAndAdvance();

		FFluxPrimeProjectileParticleData Particle;
		Particle.Position = (FVector)PositionParam.GetAndAdvance();
		Particle.Velocity = (FVector)VelocityParam.GetAndAdvance();
		Particle.Size = SizeParam.GetAndAdvance();
		Particle.Damage = DamageParam.GetAndAdvance();
		Particle.ShotId = ShotIdParam.GetAndAdvance();

		const bool bQueued = bStore && InstanceData->bCanExport && InstanceData->GatheredData.Enqueue(Particle);
		OutWasExported.SetAndAdvance(bQueued);
	}
}
