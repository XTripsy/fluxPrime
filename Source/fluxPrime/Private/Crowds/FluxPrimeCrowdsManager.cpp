// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/FluxPrimeCrowdsManager.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Cores/FluxPrimeAnimationData.h"
#include "Crowds/Components/FluxPrimeCrowdsAnimationComponent.h"
#include "Crowds/Components/FluxPrimeCrowdsNetComponent.h"
#include "Crowds/Components/FluxPrimeCrowdsSpawnerComponent.h"
#include "Crowds/Components/FluxPrimeCrowdsSystemsComponent.h"
#include "Crowds/Identity/CrowdsIdentity.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"

AFluxPrimeCrowdsManager::AFluxPrimeCrowdsManager()
{
	PrimaryActorTick.bCanEverTick = true;
	SetMinNetUpdateFrequency(20);
	
	USceneComponent* sceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = sceneRoot;
	
	CrowdsNetComponent = CreateDefaultSubobject<UFluxPrimeCrowdsNetComponent>(TEXT("CrowdsNet"));
}

void AFluxPrimeCrowdsManager::ShowDebug()
{
	FVector location = FVector::ZeroVector;
	FString debugData = "";
	
	for (int i = 0; i < CrowdsActive; ++i)
	{
		location = CrowdsDatas[CrowdsDataReadIndex].CrowdsLocation[i] + (FVector::UpVector * FluxConfig::DebugLocationIdentity);
		debugData = FString::Printf(TEXT("ID Crowds: %d \n Index On Array: %d \n Type Crowds: %d"), CrowdsDatas[CrowdsDataReadIndex].CrowdsID[i], i, CrowdsDatas[CrowdsDataReadIndex].CrowdsType[i]);
		
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

void AFluxPrimeCrowdsManager::ShortCrowdsByID()
{
	FFluxPrimeCrowds& readBuffer = CrowdsDatas[CrowdsDataReadIndex];
	
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
		NetTarget[i] = readBuffer.CrowdsCurrentTargetLocationPath[tempShortedIndex];
	}
}

void AFluxPrimeCrowdsManager::PreLoading()
{
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
			FStreamableDelegate::CreateUObject(this, &AFluxPrimeCrowdsManager::Initialize)
		);
	}
	else Initialize();
}

void AFluxPrimeCrowdsManager::Initialize()
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
		context.crowdsComponents = &CrowdsComponents;
		context.crowdsCatalog = &CrowdsCatalog;
		context.crowdsAnimationSoftRef = &CrowdsAnimationSoftRef;
		context.crowdsTotal = &CrowdsTotal;
		context.crowdsDataReadIndex = &CrowdsDataReadIndex;
		context.crowdsDataShortedIndex = &CrowdsDataShortedIndex;
		context.netAcceleration = &NetAcceleration;
		context.netTarget = &NetTarget;
		context.crowdsDatas = &CrowdsDatas;
		context.hasAuthority = HasAuthority();
		context.isReplicated = IsReplicated;
		
		CrowdsDataModule.Initialize(context);
		CrowdsDataModule.InitializeCrowds();
	}
	
	// systems
	{
		FFluxPrimeCrowdsSystemsComponentContext context;
		
		context.crowdsComponents = &CrowdsComponents;
		context.managerConfiguration = ManagerConfiguration;
		
		context.gridOffset = &GridOffset;
		context.crowdsDataShortedIndex = &CrowdsDataShortedIndex;
		context.crowdsDataReadIndex = &CrowdsDataReadIndex;
		
		context.animationSystems = &AnimationSystems;
		context.boidsSystems = &BoidsSystems;
		context.crowdsRenderSystems = &CrowdsRenderSystems;
		context.damageSystems = &DamageSystems;
		context.groundHeightSystems = &GroundHeightSystems;
		context.movementSystems = &MovementSystems;
		context.navigationSystems = &NavigationSystems;
		context.spatialGridSystems = &SpatialGridSystems;
		context.proxyTargetSystems = &ProxyTargetSystems;
		context.stateMachineSystems = &StateMachineSystems;
		
		context.crowdsDatas = &CrowdsDatas;
		context.crowdsActive = &CrowdsActive;
		context.crowdsTotal = &CrowdsTotal;
		
		context.hasAuthority = HasAuthority();
		context.world = GetWorld();
		
		CrowdsSystemsModule.Initialize(context);
		CrowdsSystemsModule.InitializeSystems();
		
		/*IdleSystems.World = GetWorld();
		AbilitySystems.World = GetWorld();
		WalkSystems.World = GetWorld();*/
	}
	
	//StateMachineSystems.InitializeStateMachineSystems();
	InitializedComponentSystems();
}

void AFluxPrimeCrowdsManager::InitializeComponentCrowds()
{
	for (int i = 0; i < CrowdsCatalog.Num(); ++i)
	{
		FString name = FString::Printf(TEXT("CROWDS_%d"), i);
    		
		FAttachmentTransformRules AttachRules(
			EAttachmentRule::KeepRelative,
			EAttachmentRule::KeepRelative,
			EAttachmentRule::KeepRelative,
			false
		);
		UInstancedStaticMeshComponent* tempISMC = NewObject<UInstancedStaticMeshComponent>(this, FName(name));
		tempISMC->SetIsReplicated(false);
		tempISMC->SetStaticMesh(CrowdsMeshSoftRef[CrowdsCatalog[i].CrowdsIdentity->Identity].Get());
		tempISMC->AttachToComponent(RootComponent, AttachRules);
		tempISMC->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		tempISMC->SetCollisionProfileName(ProfileNameCollisionCrowds);
		tempISMC->ClearInstances();
		tempISMC->UpdateBounds();
		tempISMC->RecreatePhysicsState();
		tempISMC->NumCustomDataFloats = 6;
		tempISMC->bReceivesDecals = false;
		tempISMC->RegisterComponent();
		
		CrowdsComponents.Add(tempISMC);
		
		CrowdsTypes.Add(CrowdsCatalog[i].CrowdsIdentity->Identity, i);
	}
}

void AFluxPrimeCrowdsManager::InitializedComponentSystems()
{
	// Animation
	if (AnimationSystems.IsActive)
	{
		FFluxPrimeCrowdsAnimationComponentContext context;
		context.animationSystems = &AnimationSystems;
		context.crowdsComponents = &CrowdsComponents;
		context.world = GetWorld();
		
		CrowdsAnimationModule.Initialize(context);
		
		//StateMachineSystems.OnCrowdsStateChange.BindRaw(&CrowdsAnimationModule, &FFluxPrimeCrowdsAnimationModule::SwitchAnimation);
		//MovementSystems.SwitchAnimation.BindRaw(&CrowdsAnimationModule, &FFluxPrimeCrowdsAnimationModule::SwitchAnimation);
		//AbilitySystems.OnCrowdsStateChange.BindRaw(&CrowdsAnimationModule, &FFluxPrimeCrowdsAnimationModule::SwitchAnimation);
		//IdleSystems.OnCrowdsStateChange.BindRaw(&CrowdsAnimationModule, &FFluxPrimeCrowdsAnimationModule::SwitchAnimation);
		//WalkSystems.OnCrowdsStateChange.BindRaw(&CrowdsAnimationModule, &FFluxPrimeCrowdsAnimationModule::SwitchAnimation);
		AnimationSystems.OnCrowdsStateChange.BindRaw(&CrowdsAnimationModule, &FFluxPrimeCrowdsAnimationModule::SwitchAnimation);
	}
	
	// Spawner
	{
		FFluxPrimeCrowdsSpawnerComponentContext context;
		context.crowdsActive = &CrowdsActive;
		context.crowdsTotal = &CrowdsTotal;
		context.crowdsData = &CrowdsDatas[CrowdsDataReadIndex];
		context.navigationSystems = &NavigationSystems;
		context.crowdsComponents = &CrowdsComponents;
		context.crowdsTypes = &CrowdsTypes;
		context.world = GetWorld();
		
		CrowdsSpawnerModule.Initialize(context);
		
		CrowdsSpawnerModule.OnSpawnCrowdsNet.BindUObject(CrowdsNetComponent, &UFluxPrimeCrowdsNetComponent::OnSpawnCrowdsData);
	}
	
	// Net
	if (IsReplicated)
	{
		FFluxPrimeCrowdsNetComponentContext context;
		context.crowdsTotal = CrowdsTotal;
		context.crowdsComponents = CrowdsComponents;
		
		CrowdsNetComponent->Initialize(context);
	}
}

void AFluxPrimeCrowdsManager::OnRep_CrowdActive()
{
	CrowdsNetComponent->OnCrowdsActiveChange(CrowdsActive);
}

void AFluxPrimeCrowdsManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetActorLocation(FVector::ZeroVector);
}

void AFluxPrimeCrowdsManager::BeginPlay()
{
	Super::BeginPlay();
	
	SetReplicates(IsReplicated);
	
	PreLoading();
}

void AFluxPrimeCrowdsManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (CrowdsComponents.IsEmpty() || CrowdsActive <= 0) return;
	
	if (!HasAuthority() && IsReplicated)
	{
		ShortCrowdsByID();
		CrowdsNetComponent->UpdateCrowdsData(NetAcceleration, NetTarget);
		CrowdsNetComponent->UpdateNetData(DeltaTime, GroundHeightSystems, MovementSystems, CrowdsRenderSystems);
		return;
	}
	
	//if (SpatialGridSystems.IsActive) SpatialGridSystems.UpdateSpatialGridSystem(GetWorld(), CrowdsDatas, GridOffset, CrowdsDataShortedIndex, CrowdsDataReadIndex, CrowdsActive);
	//if (BoidsSystems.IsActive && SpatialGridSystems.IsActive) BoidsSystems.UpdateBoidsSystems(CrowdsDatas[CrowdsDataReadIndex], GridOffset, CrowdsActive);
	//if (HasAuthority() && IsReplicated) ShortCrowdsByID(); 
	//if (GroundHeightSystems.IsActive) GroundHeightSystems.UpdateGroundHeightSystems(DeltaTime, CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	//if (NavigationSystems.IsActive) NavigationSystems.UpdateNavigationSystems(CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	//if (MovementSystems.IsActive) MovementSystems.UpdateMovementSystems(GetWorld(), DeltaTime, CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	//if (AnimationSystems.IsActive) AnimationSystems.UpdateAnimationSystemsFrame(GetWorld(), CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	
	//StateMachineSystems.UpdateStateMachineSystems(CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	CrowdsSystemsModule.TickSystems(DeltaTime);
	/*AbilitySystems.UpdateAbilitySystems(CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	IdleSystems.UpdateIdleSystems(CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	WalkSystems.UpdateWalkSystems(CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);*/
	
	if (IsShowDebug) ShowDebug();
	
	ForceNetUpdate();
	//CrowdsRenderSystems.UpdateRenderCrowdsSystems(CrowdsComponents, CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
}

void AFluxPrimeCrowdsManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (!HasAuthority() && IsReplicated)
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

void AFluxPrimeCrowdsManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFluxPrimeCrowdsManager, CrowdsActive);
	DOREPLIFETIME(AFluxPrimeCrowdsManager, NetAcceleration);
	DOREPLIFETIME(AFluxPrimeCrowdsManager, NetTarget);
}

void AFluxPrimeCrowdsManager::TakeDamage_Implementation(UCrowdsIdentity* Identity)
{
	// sementara
	if (DamageSystems.IsActive) DamageSystems.TakeDamage(GetWorld(), &SpatialGridSystems,CrowdsDatas[CrowdsDataReadIndex], 0, CrowdsActive);
}
