// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/FluxPrimeCrowdsManager.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Cores/FluxPrimeAnimationData.h"
#include "Crowds/Identity/CrowdsIdentity.h"
#include "Engine/AssetManager.h"

AFluxPrimeCrowdsManager::AFluxPrimeCrowdsManager()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AFluxPrimeCrowdsManager::InitializeComponentCrowds()
{
	FStreamableManager& streamable = UAssetManager::GetStreamableManager();
	
	for (int i = 0; i < CatalogCrowds.Num(); ++i)
	{
		streamable.RequestAsyncLoad(CatalogCrowds[i].CrowdsIdentity->Mesh.ToSoftObjectPath(), FStreamableDelegate::CreateLambda(
			[this, i]()
			{
				UStaticMesh* loadedMesh = CatalogCrowds[i].CrowdsIdentity->Mesh.Get(); 
				if (loadedMesh)
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
					tempISMC->SetStaticMesh(loadedMesh);
					tempISMC->AttachToComponent(RootComponent, AttachRules);
					tempISMC->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
					tempISMC->RegisterComponent();
					tempISMC->ClearInstances();
					tempISMC->UpdateBounds();
					tempISMC->RecreatePhysicsState();
					tempISMC->NumCustomDataFloats = 2;
					tempISMC->bReceivesDecals = false;
			        
					CrowdsComponents.Add(tempISMC);
					
					CrowdsTypes.Add(CatalogCrowds[i].CrowdsIdentity, i);
					
					if (i == CatalogCrowds.Num()-1 && HasAuthority()) InitializeCrowds();
				}
			}));
	}
}

void AFluxPrimeCrowdsManager::InitializeCrowds()
{
	FStreamableManager& streamable = UAssetManager::GetStreamableManager();
	
	for (int i = 0; i < CatalogCrowds.Num(); ++i)
	{
		CrowdsTotal += CatalogCrowds[i].CrowdsTotal;
	}
	
	CrowdsDatas[CrowdsDataReadIndex].Init(CrowdsTotal);
	CrowdsDatas[CrowdsDataReadIndex+1].Init(CrowdsTotal);
	CrowdsDataShortedIndex.SetNumUninitialized(CrowdsTotal);
	
	for (int i = 0; i < CatalogCrowds.Num(); ++i)
	{
		for (int j = 0; j < CatalogCrowds[i].CrowdsTotal; ++j)
		{
			FTransform tempTransform;
			tempTransform.SetLocation(FVector::DownVector * 1000.0f);
			int32 id = CrowdsComponents[i]->AddInstance(tempTransform, false);
			
			CrowdsDatas[CrowdsDataReadIndex].CrowdsLocation.Add(tempTransform.GetLocation());
			CrowdsDatas[CrowdsDataReadIndex].CrowdsRotation.Add(0);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsVelocity.Add(FVector::ZeroVector);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsAcceleration.Add(FVector::ZeroVector);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsID.Add(id);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsType.Add(i);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsCellID.Add(-1);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsMaxSpeed.Add(CatalogCrowds[i].CrowdsIdentity->Speed);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsHealth.Add(CatalogCrowds[i].CrowdsIdentity->Health);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsDamage.Add(CatalogCrowds[i].CrowdsIdentity->Damage);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsSize.Add(CatalogCrowds[i].CrowdsIdentity->Size);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsNavigationPath.Add(FFluxCrowdsPath());
			CrowdsDatas[CrowdsDataReadIndex].CrowdsTargetLocation.Add(FVector::ZeroVector);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsIndexNavigationPath.Add(0);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsTotalNavigationPath.Add(0);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsAnimationData.Add(FFluxCrowdsAnimation());
			CrowdsDatas[CrowdsDataReadIndex].CrowdsAnimationIndex.Add(0);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsStartTimeAnimationFrame.Add(0);
			
			streamable.RequestAsyncLoad(CatalogCrowds[i].CrowdsIdentity->AnimationData.ToSoftObjectPath(), FStreamableDelegate::CreateLambda(
			[this, i, j]()
			{
				UFluxPrimeAnimationData* loadedData = CatalogCrowds[i].CrowdsIdentity->AnimationData.Get(); 
				if (loadedData)
				{
					if (!CrowdsDatas[CrowdsDataReadIndex].CrowdsAnimationData.IsValidIndex(j)) return;
					FFluxCrowdsAnimation animData = FFluxCrowdsAnimation();

					for (int n = 0; n < loadedData->AnimationNotifies.Num(); ++n)
					{
						for (int m = 0; m < 8; ++m)
						{
							animData.AnimationNotify[n].AnimationNotifyType[m] = loadedData->AnimationNotifies[n].AnimationNotifyType[m];
							animData.AnimationNotify[n].AnimationNotifyFrame[m] = loadedData->AnimationNotifies[n].AnimationNotifyFrame[m];
						}
					}
					
					int32 index = 0;
					for (int k = 0; k < loadedData->Animations.Num(); ++k)
					{
						animData.AnimationOffset[index] = loadedData->Animations[k].StartFrame;
						animData.AnimationOffset[index + 1] = loadedData->Animations[k].EndFrame;
						animData.AnimationLoop[index] = loadedData->AnimationLoops[k];
						animData.AnimationLoop[index + 1] = loadedData->AnimationLoops[k];
						index += 2;
					}
					
					CrowdsDatas[CrowdsDataReadIndex].CrowdsAnimationData[j] = animData;
				}
				
			}));
		}
	}
	
	CrowdsDatas[CrowdsDataReadIndex+1] = CrowdsDatas[CrowdsDataReadIndex];
}

void AFluxPrimeCrowdsManager::UpdateRenderCrowds()
{
	int32 totalComponents = CrowdsComponents.Num();
	//TArray<TArray<FTransform>> transformsPerComponent;
	//transformsPerComponent.SetNum(totalComponents);
	
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
		
		//transformsPerComponent[typeIndex].Add(transform);
	}

	for (int32 i = 0; i < totalComponents; ++i)
	{
		CrowdsComponents[i]->MarkRenderStateDirty();
	}
	
	/*for (int32 i = 0; i < totalComponents; ++i)
	{
		if (transformsPerComponent[i].Num() <= 0) continue;
		
		CrowdsComponents[i]->BatchUpdateInstancesTransforms(0, transformsPerComponent[i], true, true, true);
	}*/
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

void AFluxPrimeCrowdsManager::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeComponentCrowds();
	
	if (!HasAuthority()) return;
	
	AnimationSystems.OnAttackNotify.BindUObject(this, &AFluxPrimeCrowdsManager::OnAttackNotify);
	AnimationSystems.OnSpawnSFXNotify.BindUObject(this, &AFluxPrimeCrowdsManager::OnSpawnSFXNotify);
	AnimationSystems.OnSpawnVFXNotify.BindUObject(this, &AFluxPrimeCrowdsManager::OnSpawnVFXNotify);
	
	GroundHeightSystems.InitializedGroundHeightSystems(100.0f, FVector(-6000.0f, -6000.0f, 0), 400, 400);
	GroundHeightSystems.BakeGroundHeightSystems(GetWorld());
	SpatialGridSystems.InitializeSpatialGridSystem(10.0f, 100);
	BoidsSystems.InitializeBoidsSystems(150.0f);
}

void AFluxPrimeCrowdsManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!HasAuthority()) return;
	if (CrowdsComponents.IsEmpty() || CrowdsActive <= 0) return;
	
	SpatialGridSystems.UpdateSpatialGridSystem(CrowdsDatas, GridOffset, CrowdsDataShortedIndex, CrowdsDataReadIndex, CrowdsActive);
	BoidsSystems.UpdateBoidsSystems(CrowdsDatas[CrowdsDataReadIndex], GridOffset, 10.0f, 100, CrowdsActive);
	GroundHeightSystems.UpdateGroundHeightSystems(DeltaTime, CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	NavigationSystems.UpdateNavigationSystems(CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	MovementSystems.UpdateMovementSystems(DeltaTime, CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	AnimationSystems.UpdateAnimationSystemsFrame(GetWorld(), CrowdsDatas, CrowdsDataShortedIndex, CrowdsDataReadIndex, CrowdsActive, CrowdsComponents);
	
	UpdateRenderCrowds();
}

void AFluxPrimeCrowdsManager::SpawnCrowd_Implementation(UCrowdsIdentity* identity, 
	FVector location, FRotator rotation)
{
	if (!HasAuthority()) return;
	
	int32 indexSelected = CrowdsActive;
	int32 typeCrowds = CrowdsTypes[identity];
	
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

	TArray<FVector> path = NavigationSystems.GetNavigationPath(GetWorld(), location, targetLocation->GetActorLocation());
	int8 total = FMath::Min(path.Num() - 1, 8);
    
	for (int8 i = 0; i < total; ++i)
	{
		path[i+1].Z = 0;
		CrowdsDatas[CrowdsDataReadIndex].CrowdsNavigationPath[indexSelected].LocationPaths[i] = path[i+1];
		DrawDebugLine(GetWorld(), path[i+1] + (FVector::UpVector * 1000), path[i+1], FColor::Blue, true, 10.0f, 2, 2.0f);
	}

	// debug
	for (int i = 0; i < path.Num(); ++i)
	{
		DrawDebugLine(GetWorld(), path[i] + (FVector::UpVector * 1000), path[i], FColor::Yellow, true, 100.0f, 1, 2.0f);
	}
	// end debug
	
	CrowdsDatas[CrowdsDataReadIndex].CrowdsLocation[indexSelected] = FVector(location.X, location.Y, 0);
	CrowdsDatas[CrowdsDataReadIndex].CrowdsRotation[indexSelected] = FRotator::CompressAxisToByte(rotation.Yaw);
	CrowdsDatas[CrowdsDataReadIndex].CrowdsID[indexSelected] = id;
	CrowdsDatas[CrowdsDataReadIndex].CrowdsTargetLocation[indexSelected] = FVector(targetLocation->GetActorLocation().X, targetLocation->GetActorLocation().Y, 0);
	CrowdsDatas[CrowdsDataReadIndex].CrowdsIndexNavigationPath[indexSelected] = 0;
	CrowdsDatas[CrowdsDataReadIndex].CrowdsTotalNavigationPath[indexSelected] = total;
	CrowdsDatas[CrowdsDataReadIndex].CrowdsStartTimeAnimationFrame[indexSelected] = GetWorld()->GetRealTimeSeconds();
	CrowdsDatas[CrowdsDataReadIndex].CrowdsAnimationIndex[indexSelected] = 0;
	
	++CrowdsActive;
}

void AFluxPrimeCrowdsManager::SwitchAnimationCrowd_Implementation(UCrowdsIdentity* identity)
{
	int32 indexCrowds = 0;
	AnimationSystems.SwitchAnimation(GetWorld(), CrowdsDatas[CrowdsDataReadIndex], indexCrowds);
}

void AFluxPrimeCrowdsManager::PlayMontageCrowd_Implementation(UCrowdsIdentity* identity)
{
	int32 indexCrowds = 0;
	AnimationSystems.MontageAnimation(CrowdsDatas[CrowdsDataReadIndex], indexCrowds, 4);
}
