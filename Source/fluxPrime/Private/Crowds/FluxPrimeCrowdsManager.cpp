// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/FluxPrimeCrowdsManager.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Cores/FluxPrimeAnimationData.h"
#include "Crowds/Components/FluxPrimeCrowdsAnimationComponent.h"
#include "Crowds/Components/FluxPrimeCrowdsNetComponent.h"
#include "Crowds/Components/FluxPrimeCrowdsSpawnerComponent.h"
#include "Crowds/Identity/CrowdsIdentity.h"
#include "Crowds/ManagerConfiguration/ManagerConfiguration.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationAnimationSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationBoidsSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationDamageSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationGroundHeightSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationMovementSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationNavigationSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationProxyTargetSystems.h"
#include "Crowds/ManagerConfiguration/Configurations/FluxPrimeConfigurationSpatialGridSystems.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"

AFluxPrimeCrowdsManager::AFluxPrimeCrowdsManager()
{
	PrimaryActorTick.bCanEverTick = true;
	SetMinNetUpdateFrequency(20);
	
	USceneComponent* sceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = sceneRoot;
	
	SpawnerComponent = CreateDefaultSubobject<UFluxPrimeCrowdsSpawnerComponent>(TEXT("SpawnerSystem"));
	AnimationComponent = CreateDefaultSubobject<UFluxPrimeCrowdsAnimationComponent>(TEXT("AnimationSystem"));
	CrowdsNetComponent = CreateDefaultSubobject<UFluxPrimeCrowdsNetComponent>(TEXT("CrowdsNet"));
}

void AFluxPrimeCrowdsManager::ShowDebug()
{
	FVector location = FVector::ZeroVector;
	FString debugData = "";
	
	for (int i = 0; i < CrowdsActive; ++i)
	{
		location = CrowdsDatas[CrowdsDataReadIndex].CrowdsLocation[i] + (FVector::UpVector * FluxConfig::DebugLocationIdentity);
		debugData = FString::Printf(TEXT("ID Crowds: %d \n Index On Array: %d"), CrowdsDatas[CrowdsDataReadIndex].CrowdsID[i], i);
		
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
	InitializeCrowds();
	
	InitializeSystems();
	InitializedComponentSystems();
}

void AFluxPrimeCrowdsManager::InitializeSystems()
{
	const FFluxPrimeConfigurationGroundHeightSystems* configurationGroundHeight = nullptr;
	const FFluxPrimeConfigurationSpatialGridSystems* configurationSpatialGrid = nullptr;
	const FFluxPrimeConfigurationNavigationSystems* configurationNavigation = nullptr;
	const FFluxPrimeConfigurationBoidsSystems* configurationBoids = nullptr;
	const FFluxPrimeConfigurationMovementSystems* configurationMovement = nullptr;
	const FFluxPrimeConfigurationAnimationSystems* configurationAnimation = nullptr;
	const FFluxPrimeConfigurationProxyTargetSystems* configurationProxyTarget = nullptr;
	const FFluxPrimeConfigurationDamageSystems* configurationDamage = nullptr;
	
	for (auto& pair : ManagerConfiguration->ConfigurationFragments)
	{
		if (!pair.IsValid()) continue;
		
		if (const FFluxPrimeConfigurationGroundHeightSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationGroundHeightSystems>())
		{
			GroundHeightSystems.IsActive = true;
			configurationGroundHeight = configuration;
			continue;
		}
		
		if (const FFluxPrimeConfigurationSpatialGridSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationSpatialGridSystems>())
		{
			if (!HasAuthority()) continue;
			SpatialGridSystems.IsActive = true;
			configurationSpatialGrid = configuration;
			continue;
		}
		
		if (const FFluxPrimeConfigurationNavigationSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationNavigationSystems>())
		{
			if (!HasAuthority()) continue;
			NavigationSystems.IsActive = true;
			configurationNavigation = configuration;
			continue;
		}
		
		if (const FFluxPrimeConfigurationBoidsSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationBoidsSystems>())
		{
			if (!HasAuthority()) continue;
			BoidsSystems.IsActive = true;
			configurationBoids = configuration;
			continue;
		}
		
		if (const FFluxPrimeConfigurationMovementSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationMovementSystems>())
		{
			MovementSystems.IsActive = true;
			configurationMovement = configuration;
			continue;
		}
		
		if (const FFluxPrimeConfigurationAnimationSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationAnimationSystems>())
		{
			if (!HasAuthority()) continue;
			AnimationSystems.IsActive = true;
			configurationAnimation = configuration;
			continue;
		}
		
		if (const FFluxPrimeConfigurationProxyTargetSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationProxyTargetSystems>())
		{
			if (!HasAuthority()) continue;
			ProxyTargetSystems.IsActive = true;
			configurationProxyTarget = configuration;
			continue;
		}
		
		if (const FFluxPrimeConfigurationDamageSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationDamageSystems>())
		{
			if (!HasAuthority()) continue;
			DamageSystems.IsActive = true;
			configurationDamage = configuration;
			continue;
		}
	}
	
#if !WITH_EDITOR
	IsShowDebug = false;
	if (configurationSpatialGrid) configurationSpatialGrid->IsDebug = false;
	if (configurationNavigation) configurationNavigation->IsDebug = false;
	if (configurationAnimation) configurationAnimation->IsDebug = false;
	if (configurationMovement) configurationMovement->IsDebug = false;
#endif
	
	if (SpatialGridSystems.IsActive) SpatialGridSystems.InitializedSpatialGridSystems(configurationSpatialGrid->IsDebug, configurationSpatialGrid->CellSize, configurationSpatialGrid->Origin, configurationSpatialGrid->CellWidth, configurationSpatialGrid->CellHeight);
	if (SpatialGridSystems.IsActive) SpatialGridSystems.BakeSpatialGridSystems(GetWorld());
	if (GroundHeightSystems.IsActive) GroundHeightSystems.InitializedGroundHeightSystems(configurationGroundHeight->CellSize, configurationGroundHeight->Origin, configurationGroundHeight->CellWidth, configurationGroundHeight->CellHeight);
	if (GroundHeightSystems.IsActive) GroundHeightSystems.BakeGroundHeightSystems(GetWorld());
	if (NavigationSystems.IsActive) NavigationSystems.InitializedNavigationSystems(configurationNavigation->IsDebug, GetWorld());
	if (BoidsSystems.IsActive && SpatialGridSystems.IsActive) BoidsSystems.InitializeBoidsSystems(configurationBoids->SeparationWeight, configurationSpatialGrid->CellSize, configurationSpatialGrid->Origin, configurationSpatialGrid->CellWidth, configurationSpatialGrid->CellHeight);
	if (ProxyTargetSystems.IsActive) ProxyTargetSystems.InitializedProxyTargetSystems(GetWorld(), &CrowdsDatas, &CrowdsActive, &NavigationSystems);
	if (AnimationSystems.IsActive) AnimationSystems.InitializedAnimationSystems(configurationAnimation->IsDebug, CrowdsComponents);
	if (MovementSystems.IsActive) MovementSystems.InitializedMovementSystems(configurationMovement->IsDebug);
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
		tempISMC->RegisterComponent();
		tempISMC->ClearInstances();
		tempISMC->UpdateBounds();
		tempISMC->RecreatePhysicsState();
		tempISMC->NumCustomDataFloats = 3;
		tempISMC->bReceivesDecals = false;
		
		CrowdsComponents.Add(tempISMC);
		
		CrowdsTypes.Add(CrowdsCatalog[i].CrowdsIdentity->Identity, i);
	}
}

void AFluxPrimeCrowdsManager::InitializedComponentSystems()
{
	if (AnimationSystems.IsActive) AnimationComponent->Initialize(AnimationSystems);
	SpawnerComponent->Initialize(&CrowdsActive, &CrowdsTotal, CrowdsComponents, CrowdsTypes, &CrowdsDatas[CrowdsDataReadIndex], &NavigationSystems);
	if (IsReplicated) CrowdsNetComponent->Initialize(CrowdsTotal, CrowdsComponents);
	SpawnerComponent->OnSpawnCrowdsNet.BindUObject(CrowdsNetComponent, &UFluxPrimeCrowdsNetComponent::OnSpawnCrowdsData);
}

void AFluxPrimeCrowdsManager::InitializeCrowds()
{
	for (int i = 0; i < CrowdsCatalog.Num(); ++i)
	{
		CrowdsTotal += CrowdsCatalog[i].CrowdsTotal;
	}
	
	CrowdsDatas[CrowdsDataReadIndex].Init(CrowdsTotal);
	CrowdsDatas[CrowdsDataReadIndex+1].Init(CrowdsTotal);
	CrowdsDataShortedIndex.SetNumUninitialized(CrowdsTotal);
	NetAcceleration.Init(FVector(), CrowdsTotal);
	NetTarget.Init(FVector(), CrowdsTotal);
	
	for (int i = 0; i < CrowdsCatalog.Num(); ++i)
	{
		for (int j = 0; j < CrowdsCatalog[i].CrowdsTotal; ++j)
		{
			FTransform tempTransform;
			tempTransform.SetLocation(FVector::DownVector * 1000.0f);
			int32 id = CrowdsComponents[i]->AddInstance(tempTransform, false);
			
			if (!HasAuthority() && IsReplicated) continue;
			
			FFluxCrowdsAnimation animationData = FFluxCrowdsAnimation();

			TSoftObjectPtr<UFluxPrimeAnimationData> loadedData = CrowdsAnimationSoftRef[CrowdsCatalog[i].CrowdsIdentity->Identity];
			
			for (int n = 0; n < loadedData->AnimationNotifies.Num(); ++n)
			{
				for (int m = 0; m < FluxConfig::AnimationArrayCount; ++m)
				{
					animationData.AnimationNotify[n].AnimationNotifyType[m] = loadedData->AnimationNotifies[n].AnimationNotifyType[m];
					animationData.AnimationNotify[n].AnimationNotifyFrame[m] = loadedData->AnimationNotifies[n].AnimationNotifyFrame[m];
				}
			}
			
			int32 index = 0;
			for (int k = 0; k < loadedData->Animations.Num(); ++k)
			{
				animationData.AnimationOffset[index] = loadedData->Animations[k].StartFrame;
				animationData.AnimationOffset[index + 1] = loadedData->Animations[k].EndFrame;
				animationData.AnimationLoop[index] = loadedData->AnimationLoops[k];
				animationData.AnimationLoop[index + 1] = loadedData->AnimationLoops[k];
				index += 2;
			}
			
			CrowdsDatas[CrowdsDataReadIndex].CrowdsLocation.Add(tempTransform.GetLocation());
			CrowdsDatas[CrowdsDataReadIndex].CrowdsRotation.Add(0);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsVelocity.Add(FVector::ZeroVector);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsAcceleration.Add(FVector::ZeroVector);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsID.Add(id);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsType.Add(i);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsCellID.Add(-1);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsMaxSpeed.Add(CrowdsCatalog[i].CrowdsIdentity->Speed);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsHealth.Add(CrowdsCatalog[i].CrowdsIdentity->Health);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsDamage.Add(CrowdsCatalog[i].CrowdsIdentity->Damage);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsSize.Add(CrowdsCatalog[i].CrowdsIdentity->Size);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsNavigationPath.Add(FFluxCrowdsPath());
			CrowdsDatas[CrowdsDataReadIndex].CrowdsTargetLocation.Add(FVector::ZeroVector);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsCurrentTargetLocationPath.Add(FVector::ZeroVector);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsIndexNavigationPath.Add(0);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsTotalNavigationPath.Add(0);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsAnimationData.Add(animationData);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsAnimationIndex.Add(0);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsStartTimeAnimationFrame.Add(0);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsCurrentAnimationFrame.Add(0);
		}
	}
	
	CrowdsDatas[CrowdsDataReadIndex+1] = CrowdsDatas[CrowdsDataReadIndex];
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
		CrowdsNetComponent->UpdateCrowdsData(NetAcceleration, NetTarget);
		CrowdsNetComponent->UpdateNetData(DeltaTime, GroundHeightSystems, MovementSystems, CrowdsRenderSystems);
		return;
	}
	
	if (SpatialGridSystems.IsActive) SpatialGridSystems.UpdateSpatialGridSystem(GetWorld(), CrowdsDatas, GridOffset, CrowdsDataShortedIndex, CrowdsDataReadIndex, CrowdsActive);
	if (BoidsSystems.IsActive && SpatialGridSystems.IsActive) BoidsSystems.UpdateBoidsSystems(CrowdsDatas[CrowdsDataReadIndex], GridOffset, CrowdsActive);
	if (HasAuthority() && IsReplicated) ShortCrowdsByID();
	if (GroundHeightSystems.IsActive) GroundHeightSystems.UpdateGroundHeightSystems(DeltaTime, CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	if (NavigationSystems.IsActive) NavigationSystems.UpdateNavigationSystems(CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	if (MovementSystems.IsActive) MovementSystems.UpdateMovementSystems(GetWorld(), DeltaTime, CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	if (AnimationSystems.IsActive) AnimationSystems.UpdateAnimationSystemsFrame(GetWorld(), CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	
	if (IsShowDebug) ShowDebug();
	
	ForceNetUpdate();
	CrowdsRenderSystems.UpdateRenderCrowdsSystems(CrowdsComponents, CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
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
	
	if (ProxyTargetSystems.IsActive) ProxyTargetSystems.EndPlayProxyTargetSystems();
	if (NavigationSystems.IsActive) NavigationSystems.EndPlayNavigationSystems();
	if (AnimationSystems.IsActive) AnimationSystems.EndPlayAnimationSystems();
	
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
