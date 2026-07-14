// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/Components/FluxPrimeCrowdsManager.h"

#include "Crowds/Components/FluxPrimeCrowdsNetComponent.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

UFluxPrimeCrowdsManager::UFluxPrimeCrowdsManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFluxPrimeCrowdsManager::InitializeManager(FFluxPrimeCrowdsManagerContext context)
{
	CrowdsComponents = context.CrowdsComponents;
	CrowdsNetComponent = context.CrowdsNetComponent;
	IsReplicated = context.isReplicated;
}

void UFluxPrimeCrowdsManager::ShowDebug()
{
	FVector location = FVector::ZeroVector;
	FString debugData = "";
	
	for (int i = 0; i < CrowdsActive; ++i)
	{
		//location = CrowdsDatas[CrowdsDataReadIndex].CrowdsLocation[i] + (FVector::UpVector * FluxConfig::DebugLocationIdentity);
		location = CrowdsDatas.CrowdsLocation[i] + (FVector::UpVector * FluxConfig::DebugLocationIdentity);
		//debugData = FString::Printf(TEXT("ID Crowds: %d \n Index On Array: %d \n Type Crowds: %d"), CrowdsDatas[CrowdsDataReadIndex].CrowdsID[i], i, CrowdsDatas[CrowdsDataReadIndex].CrowdsType[i]);
		debugData = FString::Printf(TEXT("ID Crowds: %d \n Index On Array: %d \n Type Crowds: %d"), CrowdsDatas.CrowdsID[i], i, CrowdsDatas.CrowdsType[i]);
		
		DrawDebugString(
			GetWorld(),
			location,
			debugData,
			nullptr,
			FColor::Red,
			0.0f,
			false,
			FluxConfig::DebugScaleFont
		);
	}
}

void UFluxPrimeCrowdsManager::ShortCrowdsByID()
{
	/*FFluxPrimeCrowds& readBuffer = CrowdsDatas[CrowdsDataReadIndex];
	
	CrowdsDataShortedIndex.SetNumUninitialized(CrowdsActive, EAllowShrinking::No);
	for (int i = 0; i < CrowdsActive; ++i)
	{
		CrowdsDataShortedIndex[i] = i;
	}
        
	Algo::Sort(CrowdsDataShortedIndex, [&readBuffer](int32 a, int32 b)
		{
			if (readBuffer.CrowdsType[a] != readBuffer.CrowdsType[b])
			{
				return readBuffer.CrowdsType[a] < readBuffer.CrowdsType[b];
			}
	        
			return readBuffer.CrowdsID[a] < readBuffer.CrowdsID[b];
		}
	);
	
	// perlu di ganti
	for (int i = 0; i < CrowdsActive; ++i)
	{
		int32 tempShortedIndex = CrowdsDataShortedIndex[i];
		
		NetAcceleration[i] = readBuffer.CrowdsAcceleration[tempShortedIndex];
		//NetTarget[i] = readBuffer.CrowdsCurrentTargetLocationPath[tempShortedIndex];
	}*/
}

void UFluxPrimeCrowdsManager::PreLoading()
{
	if (!CrowdsNetComponent) return;
	
	TArray<FSoftObjectPath> pathsToLoad;

	for (int i = 0; i < CrowdsCatalog.Num(); ++i)
	{
		if (!CrowdsCatalog[i].CrowdsIdentity->Mesh.IsPending()) continue;
		pathsToLoad.Add(CrowdsCatalog[i].CrowdsIdentity->Mesh.ToSoftObjectPath());
		
		if (!CrowdsCatalog[i].CrowdsIdentity->AnimationData.IsPending()) continue;
		pathsToLoad.Add(CrowdsCatalog[i].CrowdsIdentity->AnimationData.ToSoftObjectPath());
	}
	
	if (!pathsToLoad.IsEmpty())
	{
		FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
		StreamingHandle = StreamableManager.RequestAsyncLoad(
			pathsToLoad, 
			FStreamableDelegate::CreateUObject(this, &UFluxPrimeCrowdsManager::Initialize)
		);
	}
	else Initialize();
}

void UFluxPrimeCrowdsManager::Initialize()
{
	for (auto& temp : CrowdsCatalog)
	{
		if (UStaticMesh* meshCached = temp.CrowdsIdentity->Mesh.Get())
		{
			CrowdsMeshSoftRef.Add(temp.CrowdsIdentity->Identity, meshCached);
			UE_LOG(LogTemp, Error, TEXT("SUCESS MESH LOAD"));
		}

		if (UFluxPrimeAnimationData* animationCached = temp.CrowdsIdentity->AnimationData.Get())
		{
			CrowdsAnimationSoftRef.Add(temp.CrowdsIdentity->Identity, animationCached);
			UE_LOG(LogTemp, Error, TEXT("SUCESS ANIM LOAD"));
		}
	}
	
	if (CrowdsMeshSoftRef.IsEmpty() || CrowdsAnimationSoftRef.IsEmpty()) return;
	
	InitializeComponentCrowds();
	StreamingHandle.Reset();
	
	// Crowds
	{
		FFluxPrimeCrowdsDataComponentContext context;
		context.crowdsComponents = CrowdsComponents;
		context.crowdsCatalog = &CrowdsCatalog;
		context.crowdsAnimationSoftRef = &CrowdsAnimationSoftRef;
		context.crowdsTotal = &CrowdsTotal;
		context.crowdsLookup = &CrowdsLookup;
		context.netAcceleration = &NetAcceleration;
		context.netTarget = &NetTarget;
		context.crowdsDatas = &CrowdsDatas;
		context.hasAuthority = GetOwner()->HasAuthority();
		context.isReplicated = IsReplicated;
		
		CrowdsDataModule.Initialize(context);
		CrowdsDataModule.InitializeCrowds();
	}
	
	// systems
	{
		FFluxPrimeCrowdsSystemsComponentContext context;
		
		context.crowdsComponents = CrowdsComponents;
		context.managerConfiguration = ManagerConfiguration;
		context.crowdsCatalog = &CrowdsCatalog;
		context.crowdsAnimationSoftRef = &CrowdsAnimationSoftRef;
		
		context.crowdsDatas = &CrowdsDatas;
		context.crowdsActive = &CrowdsActive;
		context.crowdsTotal = &CrowdsTotal;
		context.crowdsLookup = &CrowdsLookup;
		
		context.hasAuthority = GetOwner()->HasAuthority();
		context.world = GetWorld();
		
		CrowdsSystemsModule.Initialize(context);
		CrowdsSystemsModule.InitializeSystems();
	}
	
	InitializedComponentSystems();
}

void UFluxPrimeCrowdsManager::InitializeComponentCrowds()
{
	for (int i = 0; i < CrowdsCatalog.Num(); ++i)
	{
		//FString name = FString::Printf(TEXT("CROWDS_%d"), i);
		FName name = CrowdsCatalog[i].CrowdsIdentity->Identity;
    		
		FAttachmentTransformRules AttachRules(
			EAttachmentRule::KeepRelative,
			EAttachmentRule::KeepRelative,
			EAttachmentRule::KeepRelative,
			false
		);
		UInstancedStaticMeshComponent* tempISMC = NewObject<UInstancedStaticMeshComponent>(GetOwner(), name);
		tempISMC->SetIsReplicated(false);
		tempISMC->SetStaticMesh(CrowdsMeshSoftRef[CrowdsCatalog[i].CrowdsIdentity->Identity].Get());
		tempISMC->AttachToComponent(GetOwner()->GetRootComponent(), AttachRules);
		tempISMC->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		tempISMC->SetCollisionProfileName(ProfileNameCollisionCrowds);
		tempISMC->ClearInstances();
		tempISMC->UpdateBounds();
		tempISMC->RecreatePhysicsState();
		tempISMC->NumCustomDataFloats = 6;
		tempISMC->bReceivesDecals = false;
		tempISMC->RegisterComponent();
		CrowdsComponents->Add(tempISMC);
		
		CrowdsTypes.Add(CrowdsCatalog[i].CrowdsIdentity->Identity, i);
		//CrowdsTypes.Add(FName(name), i);
	}
}

void UFluxPrimeCrowdsManager::InitializedComponentSystems()
{
	// Spawner
	{
		FFluxPrimeCrowdsSpawnerComponentContext context;
		context.crowdsActive = &CrowdsActive;
		context.crowdsTotal = &CrowdsTotal;
		//context.crowdsData = &CrowdsDatas[CrowdsDataReadIndex];
		context.crowdsData = &CrowdsDatas;
		context.crowdsTypes = &CrowdsTypes;
		context.crowdsLookup = &CrowdsLookup;
		context.world = GetWorld();
		
		CrowdsSpawnerModule.Initialize(context);
		
		//CrowdsSpawnerModule.OnSpawnCrowdsNet.BindUObject(CrowdsNetComponent, &UFluxPrimeCrowdsNetComponent::OnSpawnCrowdsData);
	}
	
	// Net
	if (IsReplicated)
	{
		FFluxPrimeCrowdsNetComponentContext context;
		context.crowdsTotal = CrowdsTotal;
		context.crowdsComponents = *CrowdsComponents;
		
		CrowdsNetComponent->Initialize(context);
	}
}

void UFluxPrimeCrowdsManager::OnRep_CrowdActive()
{
}

void UFluxPrimeCrowdsManager::BeginPlay()
{
	Super::BeginPlay();
	
	PreLoading();
}

void UFluxPrimeCrowdsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Manager);
	
	if (CrowdsComponents->IsEmpty() || CrowdsActive <= 0) return;
	
	/*if (!HasAuthority() && IsReplicated)
	{
		ShortCrowdsByID();
		CrowdsNetComponent->UpdateCrowdsData(NetAcceleration, NetTarget);
		CrowdsNetComponent->UpdateNetData(DeltaTime, GroundHeightSystems, MovementSystems, CrowdsRenderSystems);
		return;
	}*/
	
	CrowdsSystemsModule.TickSystems(DeltaTime);
	
	if (IsShowDebug) ShowDebug();
	
	GetOwner()->ForceNetUpdate();
}

void UFluxPrimeCrowdsManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!GetOwner()->HasAuthority() && IsReplicated)
	{
		Super::EndPlay(EndPlayReason);
		return;
	}
	
	if (StreamingHandle.IsValid() && StreamingHandle->IsActive())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
	
	CrowdsMeshSoftRef.Empty();
	CrowdsAnimationSoftRef.Empty();
	
	CrowdsSystemsModule.EndPlaySystems();
	
	Super::EndPlay(EndPlayReason);
}

void UFluxPrimeCrowdsManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UFluxPrimeCrowdsManager, CrowdsActive);
	DOREPLIFETIME(UFluxPrimeCrowdsManager, NetAcceleration);
	DOREPLIFETIME(UFluxPrimeCrowdsManager, NetTarget);
}