// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/FluxPrimeCrowdsManager.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Cores/FluxPrimeAnimationData.h"
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

AFluxPrimeCrowdsManager::AFluxPrimeCrowdsManager()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
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
			FStreamableDelegate::CreateUObject(this, &AFluxPrimeCrowdsManager::InitializeSystems)
		);
	}
	else InitializeSystems();
}

void AFluxPrimeCrowdsManager::InitializeSystems()
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
	
	if (!HasAuthority() && IsReplicated) return;
	
	InitializeCrowds();

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
			GroundHeightSystems = MakeShared<FFluxPrimeGroundHeightSystems>();
			configurationGroundHeight = configuration;
			continue;
		}
		
		if (const FFluxPrimeConfigurationSpatialGridSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationSpatialGridSystems>())
		{
			SpatialGridSystems = MakeShared<FFluxPrimeSpatialGridSystems>();
			configurationSpatialGrid = configuration;
			continue;
		}
		
		if (const FFluxPrimeConfigurationNavigationSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationNavigationSystems>())
		{
			NavigationSystems = MakeShared<FFluxPrimeNavigationSystems>();
			configurationNavigation = configuration;
			continue;
		}
		
		if (const FFluxPrimeConfigurationBoidsSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationBoidsSystems>())
		{
			BoidsSystems = MakeShared<FFluxPrimeBoidsSystems>();
			configurationBoids = configuration;
			continue;
		}
		
		if (const FFluxPrimeConfigurationMovementSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationMovementSystems>())
		{
			MovementSystems = MakeShared<FFluxPrimeMovementSystems>();
			configurationMovement = configuration;
			continue;
		}
		
		if (const FFluxPrimeConfigurationAnimationSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationAnimationSystems>())
		{
			AnimationSystems = MakeShared<FFluxPrimeAnimationSystems>();
			configurationAnimation = configuration;
			continue;
		}
		
		if (const FFluxPrimeConfigurationProxyTargetSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationProxyTargetSystems>())
		{
			ProxyTargetSystems = MakeShared<FFluxPrimeProxyTargetSystems>();
			configurationProxyTarget = configuration;
			continue;
		}
		
		if (const FFluxPrimeConfigurationDamageSystems* configuration = pair.GetPtr<FFluxPrimeConfigurationDamageSystems>())
		{
			DamageSystems = MakeShared<FFluxPrimeDamageSystems>();
			configurationDamage = configuration;
			continue;
		}
	}
	
	SpatialGridSystems->InitializedSpatialGridSystems(configurationSpatialGrid->IsDebug, configurationSpatialGrid->CellSize, configurationSpatialGrid->Origin, configurationSpatialGrid->CellWidth, configurationSpatialGrid->CellHeight);
	SpatialGridSystems->BakeSpatialGridSystems(GetWorld());
	GroundHeightSystems->InitializedGroundHeightSystems(configurationGroundHeight->CellSize, configurationGroundHeight->Origin, configurationGroundHeight->CellWidth, configurationGroundHeight->CellHeight);
	GroundHeightSystems->BakeGroundHeightSystems(GetWorld());
	NavigationSystems->InitializedNavigationSystems(configurationNavigation->IsDebug);
	BoidsSystems->InitializeBoidsSystems(configurationBoids->SeparationWeight);
	ProxyTargetSystems->InitializedProxyTargetSystems(GetWorld(), &CrowdsDatas[CrowdsDataReadIndex], &CrowdsActive, NavigationSystems);
	AnimationSystems->InitializedAnimationSystems(configurationAnimation->IsDebug);
	MovementSystems->InitializedMovementSystems(configurationMovement->IsDebug);
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
		tempISMC->SetIsReplicated(true);
		tempISMC->SetStaticMesh(CrowdsMeshSoftRef[CrowdsCatalog[i].CrowdsIdentity->Identity].Get());
		tempISMC->AttachToComponent(RootComponent, AttachRules);
		tempISMC->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		tempISMC->RegisterComponent();
		tempISMC->ClearInstances();
		tempISMC->UpdateBounds();
		tempISMC->RecreatePhysicsState();
		tempISMC->NumCustomDataFloats = 2;
		tempISMC->bReceivesDecals = false;
    			        
		CrowdsComponents.Add(tempISMC);
    					
		CrowdsTypes.Add(CrowdsCatalog[i].CrowdsIdentity->Identity, i);
	}
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
	
	for (int i = 0; i < CrowdsCatalog.Num(); ++i)
	{
		for (int j = 0; j < CrowdsCatalog[i].CrowdsTotal; ++j)
		{
			FTransform tempTransform;
			tempTransform.SetLocation(FVector::DownVector * 1000.0f);
			int32 id = CrowdsComponents[i]->AddInstance(tempTransform, false);
			
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
			CrowdsDatas[CrowdsDataReadIndex].CrowdsIndexNavigationPath.Add(0);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsTotalNavigationPath.Add(0);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsAnimationData.Add(animationData);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsAnimationIndex.Add(0);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsStartTimeAnimationFrame.Add(0);
		}
	}
	
	CrowdsDatas[CrowdsDataReadIndex+1] = CrowdsDatas[CrowdsDataReadIndex];
}

void AFluxPrimeCrowdsManager::UpdateRenderCrowds()
{
	int32 totalComponents = CrowdsComponents.Num();
	
	for (int32 i = 0; i < CrowdsActive; ++i)
	{
		int8 typeIndex = CrowdsDatas[CrowdsDataReadIndex].CrowdsType[i];
		if (typeIndex < 0 || typeIndex >= totalComponents) continue;

		FTransform transform;
		transform.SetLocation(CrowdsDatas[CrowdsDataReadIndex].CrowdsLocation[i]);

		float unpackedYaw = FRotator::DecompressAxisFromByte(CrowdsDatas[CrowdsDataReadIndex].CrowdsRotation[i]);
		FRotator Rot(0.0f, unpackedYaw, 0.0f);
		transform.SetRotation(Rot.Quaternion());
		
		CrowdsComponents[typeIndex]->UpdateInstanceTransform(
			CrowdsDatas[CrowdsDataReadIndex].CrowdsID[i],
			transform,
			true,
			false,
			true
		);
	}

	for (int32 i = 0; i < totalComponents; ++i)
	{
		CrowdsComponents[i]->MarkRenderStateDirty();
	}
}

void AFluxPrimeCrowdsManager::OnAttackNotify(int32 memberID)
{
	UE_LOG(LogTemp, Error, TEXT("[ACTION] :: ATTACK : %d"), memberID);
	
	FVector location = FVector::ZeroVector;
	FVector forwardVector = FVector::ZeroVector;

	for (int i = 0; i < CrowdsActive; ++i)
	{
		if (CrowdsDatas[CrowdsDataReadIndex].CrowdsID[i] != memberID) continue;
		
		location = CrowdsDatas[CrowdsDataReadIndex].CrowdsLocation[i] + (FVector::UpVector * 150) + (FVector::RightVector * -100);
		float unpackedYaw = FRotator::DecompressAxisFromByte(CrowdsDatas[CrowdsDataReadIndex].CrowdsRotation[i]) + 65;
		forwardVector = FRotator(0.0f, unpackedYaw, 0.0f).Vector();
	}
	
	FVector endLocation = location + (forwardVector * 135);
	
	FHitResult hit;
	float sphereRadius = 50.0f;
	FCollisionShape sphereShape = FCollisionShape::MakeSphere(sphereRadius);
	FCollisionQueryParams TraceParams;
	TraceParams.bTraceComplex = false;
	
	bool bHit = GetWorld()->SweepSingleByChannel(
		hit,
		location,
		endLocation,
		FQuat::Identity,
		ECC_Visibility,
		sphereShape,
		TraceParams
	);
	
	DrawDebugSphere(GetWorld(), endLocation, sphereRadius, 12, FColor::Red, false, 2.0f);
}

void AFluxPrimeCrowdsManager::OnSpawnSFXNotify(int32 memberID)
{
	UE_LOG(LogTemp, Error, TEXT("[ACTION] :: SPAWN SFX : %d"), memberID);
}

void AFluxPrimeCrowdsManager::OnSpawnVFXNotify(int32 memberID)
{
	UE_LOG(LogTemp, Error, TEXT("[ACTION] :: SPAWN VFX : %d"), memberID);
}

void AFluxPrimeCrowdsManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetActorLocation(FVector::ZeroVector);
}

void AFluxPrimeCrowdsManager::PostInitProperties()
{
	Super::PostInitProperties();
	
	bReplicates = IsReplicated;
}

void AFluxPrimeCrowdsManager::BeginPlay()
{
	Super::BeginPlay();
	
#if !WITH_EDITOR
	IsShowDebug = false;
#endif
	
	PreLoading();
	
	if (!HasAuthority() && IsReplicated) return;
	
	if (AnimationSystems)
	{
		AnimationSystems->OnAttackNotify.BindUObject(this, &AFluxPrimeCrowdsManager::OnAttackNotify);
		AnimationSystems->OnSpawnSFXNotify.BindUObject(this, &AFluxPrimeCrowdsManager::OnSpawnSFXNotify);
		AnimationSystems->OnSpawnVFXNotify.BindUObject(this, &AFluxPrimeCrowdsManager::OnSpawnVFXNotify);
	}
}

void AFluxPrimeCrowdsManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!HasAuthority() && IsReplicated) return;
	if (CrowdsComponents.IsEmpty() || CrowdsActive <= 0) return;
	
	if (SpatialGridSystems) SpatialGridSystems->UpdateSpatialGridSystem(GetWorld(), CrowdsDatas, GridOffset, CrowdsDataShortedIndex, CrowdsDataReadIndex, CrowdsActive);
	//BoidsSystems.UpdateBoidsSystems(CrowdsDatas[CrowdsDataReadIndex], GridOffset, 10.0f, 100, CrowdsActive);
	if (GroundHeightSystems) GroundHeightSystems->UpdateGroundHeightSystems(DeltaTime, CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	if (NavigationSystems) NavigationSystems->UpdateNavigationSystems(CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	if (MovementSystems) MovementSystems->UpdateMovementSystems(GetWorld(), DeltaTime, CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	if (AnimationSystems) AnimationSystems->UpdateAnimationSystemsFrame(GetWorld(), CrowdsDatas, CrowdsDataShortedIndex, CrowdsDataReadIndex, CrowdsActive, CrowdsComponents);
	
	if (IsShowDebug) ShowDebug();
	
	UpdateRenderCrowds();
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
	
	if (ProxyTargetSystems) ProxyTargetSystems->EndPlayProxyTargetSystems();
	if (NavigationSystems) NavigationSystems->EndPlayNavigationSystems();
	if (AnimationSystems) AnimationSystems->EndPlayAnimationSystems();
	
	Super::EndPlay(EndPlayReason);
}

void AFluxPrimeCrowdsManager::SpawnCrowd_Implementation(UCrowdsIdentity* identity, FVector location, FRotator rotation)
{
	if (!HasAuthority() && IsReplicated) return;
	if (CrowdsActive == CrowdsTotal) return;
	
	int32 indexSelected = CrowdsActive;
	int32 typeCrowds = CrowdsTypes[identity->Identity];
	
	int32 id = -1;

	for (int i = indexSelected; i < CrowdsTotal; ++i)
	{
		if (CrowdsDatas[CrowdsDataReadIndex].CrowdsType[i] == typeCrowds)
		{
			int32 tempID = CrowdsDatas[CrowdsDataReadIndex].CrowdsID[indexSelected];
			id = CrowdsDatas[CrowdsDataReadIndex].CrowdsID[i];
			CrowdsDatas[CrowdsDataReadIndex].CrowdsID[i] = tempID;
			break;
		}
	}

	int8 total = 0;
	if (NavigationSystems)
	{
		TArray<FVector> path = NavigationSystems->GetNavigationPath(GetWorld(), location, FVector::ZeroVector);
		total = FMath::Min(path.Num() - 1, FluxConfig::NavigationArrayCount);
	    
		for (int8 i = 0; i < total; ++i)
		{
			path[i+1].Z = 0;
			CrowdsDatas[CrowdsDataReadIndex].CrowdsNavigationPath[indexSelected].LocationPaths[i] = path[i+1];
		}
	}
	
	CrowdsDatas[CrowdsDataReadIndex].CrowdsLocation[indexSelected] = FVector(location.X, location.Y, 0);
	CrowdsDatas[CrowdsDataReadIndex].CrowdsRotation[indexSelected] = FRotator::CompressAxisToByte(rotation.Yaw);
	CrowdsDatas[CrowdsDataReadIndex].CrowdsID[indexSelected] = id;
	CrowdsDatas[CrowdsDataReadIndex].CrowdsIndexNavigationPath[indexSelected] = 0;
	CrowdsDatas[CrowdsDataReadIndex].CrowdsTotalNavigationPath[indexSelected] = total;
	CrowdsDatas[CrowdsDataReadIndex].CrowdsStartTimeAnimationFrame[indexSelected] = GetWorld()->GetRealTimeSeconds();
	CrowdsDatas[CrowdsDataReadIndex].CrowdsAnimationIndex[indexSelected] = 0;
	
	++CrowdsActive;
}

void AFluxPrimeCrowdsManager::SwitchAnimationCrowd_Implementation(UCrowdsIdentity* identity)
{
	int32 indexCrowds = 0;
	if (AnimationSystems) AnimationSystems->SwitchAnimation(GetWorld(), CrowdsDatas[CrowdsDataReadIndex], indexCrowds);
}

void AFluxPrimeCrowdsManager::PlayMontageCrowd_Implementation(UCrowdsIdentity* identity)
{
	int32 indexCrowds = 0;
	if (AnimationSystems) AnimationSystems->MontageAnimation(CrowdsDatas[CrowdsDataReadIndex], indexCrowds, 4);
}

void AFluxPrimeCrowdsManager::TakeDamage_Implementation(UCrowdsIdentity* Identity)
{
	if (DamageSystems) DamageSystems->TakeDamage(GetWorld(), SpatialGridSystems,CrowdsDatas[CrowdsDataReadIndex], 0, CrowdsActive);
}
