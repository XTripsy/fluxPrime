#include "Framework/Core/FluxPrimeDebugComponent.h"
#include "Framework/Core/FluxPrimeBaseManager.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "DrawDebugHelpers.h"
#include "fluxPrime.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

#if !UE_BUILD_SHIPPING
namespace
{
void DebugCommand(const TArray<FString>& Args, UWorld* World, int32 Mode)
{
	if (!World) return;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		TArray<UFluxPrimeDebugComponent*> Components;
		It->GetComponents(Components);
		for (auto* Component : Components)
		{
			if (Mode == 0) Component->SetDebugEnabled(Args.IsEmpty() || Args[0].ToBool());
			else if (Mode == 1) { UE_LOG(LogFluxPrime, Log, TEXT("%s"), *Component->DescribeRuntimeSystems()); }
			else if (Args.Num() == 2) Component->SetSystemEnabled(FCString::Atoi(*Args[0]), Args[1].ToBool());
		}
	}
}
FAutoConsoleCommandWithWorldAndArgs DebugMaster(TEXT("Flux.Debug.Crowds"), TEXT("Enable debug: <0|1>"), FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& A, UWorld* W){ DebugCommand(A,W,0); }));
FAutoConsoleCommandWithWorldAndArgs DebugList(TEXT("Flux.Debug.List"), TEXT("List runtime debug entries"), FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& A, UWorld* W){ DebugCommand(A,W,1); }));
FAutoConsoleCommandWithWorldAndArgs DebugSet(TEXT("Flux.Debug.Set"), TEXT("Toggle entry: <index> <0|1>"), FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& A, UWorld* W){ DebugCommand(A,W,2); }));
}
#endif
UFluxPrimeDebugComponent::UFluxPrimeDebugComponent()
{
	PrimaryComponentTick.bCanEverTick = !UE_BUILD_SHIPPING;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}
void UFluxPrimeDebugComponent::SetDebugEnabled(bool bEnabled)
{
#if !UE_BUILD_SHIPPING
	bEnableDebug = bEnabled;
	RefreshTickEnabled();
#endif
}
bool UFluxPrimeDebugComponent::SetSystemEnabled(int32 Index, bool bEnabled)
{
#if !UE_BUILD_SHIPPING
	if (!RuntimeEntries.IsValidIndex(Index)) return false;
	RuntimeEntries[Index].bRuntimeEnabled = bEnabled;
	if (bEnabled && !RuntimeEntries[Index].bInitialized && Manager.IsValid())
	{
		const auto View = Manager->CreateDebugReadView();
		FluxPrimeInitializeDebugSystems(View, Manager->GetDebugArchetypeMeta(), RuntimeDebugSystems, MakeArrayView(&RuntimeEntries[Index], 1));
	}
	RefreshTickEnabled();
	return true;
#else
	return false;
#endif
}
void UFluxPrimeDebugComponent::BindManager(UFluxPrimeBaseManager* InManager)
{
#if !UE_BUILD_SHIPPING
	if (Manager.IsValid()) RemoveTickPrerequisiteComponent(Manager.Get());
	Manager = InManager;
	if (InManager) AddTickPrerequisiteComponent(InManager);
	RebuildRuntime();
#endif
}
void UFluxPrimeDebugComponent::BeginPlay()
{
	Super::BeginPlay();
#if !UE_BUILD_SHIPPING
	BindManager(Manager.IsValid() ? Manager.Get() : GetOwner()->FindComponentByClass<UFluxPrimeBaseManager>());
#endif
}
void UFluxPrimeDebugComponent::EndPlay(const EEndPlayReason::Type Reason)
{
#if !UE_BUILD_SHIPPING
	RuntimeEntries.Reset();
	RuntimeDebugSystems.Reset();
	Manager.Reset();
#endif
	Super::EndPlay(Reason);
}
void UFluxPrimeDebugComponent::RebuildRuntime()
{
#if !UE_BUILD_SHIPPING
	RuntimeDebugSystems.Reset();
	RuntimeEntries.Reset();
	InitializedArchetypeCount = INDEX_NONE;
	if (DebugConfiguration)
	{
		RuntimeDebugSystems = DebugConfiguration->DebugSystems;
		FluxPrimeBuildDebugRuntime(RuntimeDebugSystems, RuntimeEntries);
	}
	RefreshTickEnabled();
#endif
}
void UFluxPrimeDebugComponent::RefreshTickEnabled()
{
#if !UE_BUILD_SHIPPING
	bool bAny = false;
	for (const auto& Entry : RuntimeEntries) bAny |= Entry.bRuntimeEnabled;
	// Keep observing while data is loading; initial binding failure must not permanently disable recovery.
	SetComponentTickEnabled(bEnableDebug && Manager.IsValid() && bAny && GetNetMode() != NM_DedicatedServer);
#endif
}
FString UFluxPrimeDebugComponent::DescribeRuntimeSystems() const
{
	FString Result;
#if !UE_BUILD_SHIPPING
	for (int32 I = 0; I < RuntimeEntries.Num(); ++I)
	{
		const auto& Entry = RuntimeEntries[I];
		Result += FString::Printf(TEXT("%d %s enabled=%d valid=%d\n"), I, *GetNameSafe(Entry.Type), Entry.bRuntimeEnabled, RuntimeDebugSystems[Entry.SystemIndex].Get<FFluxPrimeBaseDebugSystem>().bIsDataValid);
	}
#endif
	return Result;
}
void UFluxPrimeDebugComponent::TickComponent(float Delta, ELevelTick TickType, FActorComponentTickFunction* TickFunction)
{
#if !UE_BUILD_SHIPPING
	if (!Manager.IsValid() || !DebugConfiguration || !bEnableDebug || GetNetMode() == NM_DedicatedServer) return;
	FFluxPrimeDebugReadView View;
	TConstArrayView<FFluxPrimeDebugArchetypeMeta> Meta;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Debug_Read);
		View = Manager->CreateDebugReadView();
		Meta = Manager->GetDebugArchetypeMeta();
	}
	if (InitializedArchetypeCount != View.NumArchetypes())
	{
		FluxPrimeInitializeDebugSystems(View, Meta, RuntimeDebugSystems, RuntimeEntries);
		InitializedArchetypeCount = View.NumArchetypes();
	}
	FVector Camera = GetOwner()->GetActorLocation();
	if (const auto* PC = GetWorld()->GetFirstPlayerController()) if (PC->PlayerCameraManager) Camera = PC->PlayerCameraManager->GetCameraLocation();
	FFluxPrimeDebugFrame Frame;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Debug_Format);
		FluxPrimeRunDebugPass(View, Meta, DebugConfiguration->CandidateLocation, DebugConfiguration->GlobalLimits, DebugConfiguration->BaseTextHeightOffset, DebugConfiguration->TextHeightStepPerSystem, Camera, RuntimeDebugSystems, RuntimeEntries, Frame);
	}
	TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Debug_Draw);
	for (const auto& Label : Frame.Labels) DrawDebugString(GetWorld(), Label.Location, Label.Text, nullptr, Label.Color, 0, true, Label.FontScale);
	for (const auto& Shape : Frame.Shapes)
	{
		switch (Shape.Kind)
		{
		case EFluxPrimeDebugShapeKind::Line: DrawDebugLine(GetWorld(), Shape.Start, Shape.End, Shape.Color, false, 0, 0, Shape.Thickness); break;
		case EFluxPrimeDebugShapeKind::Arrow: DrawDebugDirectionalArrow(GetWorld(), Shape.Start, Shape.End, Shape.Extent.X, Shape.Color, false, 0, 0, Shape.Thickness); break;
		case EFluxPrimeDebugShapeKind::Box: DrawDebugBox(GetWorld(), Shape.Center, Shape.Extent, Shape.Color, false, 0, 0, Shape.Thickness); break;
		case EFluxPrimeDebugShapeKind::Circle: DrawDebugCircle(GetWorld(), Shape.Center, Shape.Radius, 32, Shape.Color, false, 0, 0, Shape.Thickness, FVector::ForwardVector, FVector::RightVector, false); break;
		case EFluxPrimeDebugShapeKind::Point: DrawDebugPoint(GetWorld(), Shape.Center, FMath::Max(1.f, Shape.Radius), Shape.Color, false, 0); break;
		}
	}
#endif
}
