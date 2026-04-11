// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/FluxPrimeCrowdsManager.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Crowds/Identity/CrowdsIdentity.h"


AFluxPrimeCrowdsManager::AFluxPrimeCrowdsManager()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AFluxPrimeCrowdsManager::InitializeComponentCrowds()
{
	for (int i = 0; i < CatalogCrowds.Num(); ++i)
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
		tempISMC->SetStaticMesh(CatalogCrowds[i].CrowdsIdentity->Mesh);
		tempISMC->AttachToComponent(RootComponent, AttachRules);
		tempISMC->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		tempISMC->RegisterComponent();
		tempISMC->ClearInstances();
		tempISMC->UpdateBounds();
		tempISMC->RecreatePhysicsState();
        
		CrowdsComponents.Add(tempISMC);
		
		CrowdsTypes.Add(CatalogCrowds[i].CrowdsIdentity, i);
	}
}

void AFluxPrimeCrowdsManager::InitializeCrowds()
{
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
			CrowdsDatas[CrowdsDataReadIndex].CrowdsVelocity.Add(FVector::ZeroVector);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsAcceleration.Add(FVector::ZeroVector);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsNavigationPath.Add(FFluxCrowdsPath());
			CrowdsDatas[CrowdsDataReadIndex].CrowdsTargetLocation.Add(FVector::ZeroVector);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsCellID.Add(-1);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsRotation.Add(0);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsMaxSpeed.Add(CatalogCrowds[i].CrowdsIdentity->Speed);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsType.Add(i);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsID.Add(id);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsHealth.Add(CatalogCrowds[i].CrowdsIdentity->Health);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsDamage.Add(CatalogCrowds[i].CrowdsIdentity->Damage);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsSize.Add(CatalogCrowds[i].CrowdsIdentity->Size);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsIndexNavigationPath.Add(0);
			CrowdsDatas[CrowdsDataReadIndex].CrowdsTotalNavigationPath.Add(0);
		}
	}
	
	CrowdsDatas[CrowdsDataReadIndex+1] = CrowdsDatas[CrowdsDataReadIndex];
}

void AFluxPrimeCrowdsManager::UpdateRenderCrowds()
{
	int32 totalComponents = CrowdsComponents.Num();
	TArray<TArray<FTransform>> transformsPerComponent;
	transformsPerComponent.SetNum(totalComponents);
	
	for (int32 i = 0; i < CrowdsActive; ++i)
	{
		int8 typeIndex = CrowdsDatas[CrowdsDataReadIndex].CrowdsType[i];
		if (typeIndex < 0 || typeIndex >= totalComponents) continue;

		FTransform transform;
		transform.SetLocation(CrowdsDatas[CrowdsDataReadIndex].CrowdsLocation[i]);

		float unpackedYaw = FRotator::DecompressAxisFromByte(CrowdsDatas[CrowdsDataReadIndex].CrowdsRotation[i]);
		FRotator Rot(0.0f, unpackedYaw, 0.0f);
		transform.SetRotation(Rot.Quaternion());
		
		transformsPerComponent[typeIndex].Add(transform);
	}

	for (int32 i = 0; i < totalComponents; ++i)
	{
		if (transformsPerComponent[i].Num() <= 0) continue;
		
		CrowdsComponents[i]->BatchUpdateInstancesTransforms(0, transformsPerComponent[i], true, true, true);
	}
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
	
	InitializeCrowds();
	
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
	MovementSystems.UpdateMovementSystems(DeltaTime, CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	NavigationSystems.UpdateNavigationSystems(CrowdsDatas[CrowdsDataReadIndex], CrowdsActive);
	
	UpdateRenderCrowds();
}

void AFluxPrimeCrowdsManager::SpawnCrowd_Implementation(UCrowdsIdentity* identity, 
	FVector location, FRotator rotation)
{
	if (!HasAuthority()) return;
	
	int32 indexSelected = CrowdsActive;
	int32 typeCrowds = CrowdsTypes[identity];
	
	int32 id = -1;
	int32 type = -1;

	for (int i = indexSelected; i < CrowdsTotal; ++i)
	{
		if (CrowdsDatas[CrowdsDataReadIndex].CrowdsType[i] == typeCrowds)
		{
			int32 tempID = CrowdsDatas[CrowdsDataReadIndex].CrowdsID[indexSelected];
			int32 tempType = CrowdsDatas[CrowdsDataReadIndex].CrowdsType[indexSelected];
			
			id = CrowdsDatas[CrowdsDataReadIndex].CrowdsID[i];
			type = CrowdsDatas[CrowdsDataReadIndex].CrowdsType[i];
			
			CrowdsDatas[CrowdsDataReadIndex].CrowdsID[i] = tempID;
			CrowdsDatas[CrowdsDataReadIndex].CrowdsType[i] = tempType;
			break;
		}
	}

	TArray<FVector> path = NavigationSystems.GetNavigationPath(GetWorld(), location, targetLocation->GetActorLocation());
	int8 total = FMath::Min(path.Num() - 1, 8);
    
	for (int8 i = 0; i < total; ++i)
	{
		CrowdsDatas[CrowdsDataReadIndex].CrowdsNavigationPath[indexSelected].LocationPaths[i] = path[i+1];
	}
	
	CrowdsDatas[CrowdsDataReadIndex].CrowdsLocation[indexSelected] = location;
	CrowdsDatas[CrowdsDataReadIndex].CrowdsRotation[indexSelected] = FRotator::CompressAxisToByte(rotation.Yaw);
	CrowdsDatas[CrowdsDataReadIndex].CrowdsID[indexSelected] = id;
	CrowdsDatas[CrowdsDataReadIndex].CrowdsType[indexSelected] = type;
	CrowdsDatas[CrowdsDataReadIndex].CrowdsTargetLocation[indexSelected] = targetLocation->GetActorLocation();
	CrowdsDatas[CrowdsDataReadIndex].CrowdsDamage[indexSelected] = identity->Damage;
	CrowdsDatas[CrowdsDataReadIndex].CrowdsSize[indexSelected] = identity->Size;
	CrowdsDatas[CrowdsDataReadIndex].CrowdsHealth[indexSelected] = identity->Health;
	CrowdsDatas[CrowdsDataReadIndex].CrowdsMaxSpeed[indexSelected] = identity->Speed;
	CrowdsDatas[CrowdsDataReadIndex].CrowdsIndexNavigationPath[indexSelected] = 0;
	CrowdsDatas[CrowdsDataReadIndex].CrowdsTotalNavigationPath[indexSelected] = path.Num()-1;
	
	++CrowdsActive;
}

