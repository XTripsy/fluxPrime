// Fill out your copyright notice in the Description page of Project Settings.


#include "Templates/FluxPrimeCrowd.h"

#include "Components/CapsuleComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Cores/FluxPrimeStruct.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

void AFluxPrimeCrowd::OnRep_MemberData()
{
	int32 index = 0; 
    
	for (int32 componentIndex = 0; componentIndex < MemberComponents.Num(); componentIndex++)
	{
		TArray<FTransform> tempTransforms;
		int32 CountForThisMesh = MemberTypeCounts[componentIndex];
       
		for (int32 j = 0; j < CountForThisMesh; j++)
		{
			FFluxPrimeNet& CurrentNetData = MemberTransforms[index];
           
			FTransform transform;
			transform.SetLocation(CurrentNetData.NetLocation);
          
			float UnpackedYaw = FRotator::DecompressAxisFromByte(CurrentNetData.NetRotation);
			FRotator Rot = FRotator(0.0f, UnpackedYaw, 0.0f);
			transform.SetRotation(Rot.Quaternion());
          
			FVector RebuiltScale = CurrentNetData.isActive ? FVector(1.0f) : FVector::ZeroVector;
			transform.SetScale3D(RebuiltScale);
          
			tempTransforms.Add(transform);
          
			index++;
		}
       
		if (MemberComponents[componentIndex]->GetInstanceCount() == 0)
			MemberComponents[componentIndex]->AddInstances(tempTransforms, false, false);
		else
			MemberComponents[componentIndex]->BatchUpdateInstancesTransforms(0, tempTransforms, false, true, false);
	}
}

void AFluxPrimeCrowd::DisableMemeber(FString type, int32 id)
{
	if (type.IsEmpty() || id < 0 || !HasAuthority()) return;

	int32 index = 0; 
    
	for (int32 componentIndex = 0; componentIndex < MemberComponents.Num(); componentIndex++)
	{
		TArray<FTransform> tempTransforms;
		int32 CountForThisMesh = MemberTypeCounts[componentIndex];
       
		for (int32 j = 0; j < CountForThisMesh; j++)
		{
			FFluxPrimeNet& CurrentNetData = MemberTransforms[index];
			
			FString memberType = FString::Printf(TEXT("CROWD_%d"), CurrentNetData.NetType);
			if (CurrentNetData.NetID == id && memberType.Equals(type)) CurrentNetData.isActive = false;
           
			FTransform transform;
			transform.SetLocation(CurrentNetData.NetLocation);
          
			float UnpackedYaw = FRotator::DecompressAxisFromByte(CurrentNetData.NetRotation);
			FRotator Rot = FRotator(0.0f, UnpackedYaw, 0.0f);
			transform.SetRotation(Rot.Quaternion());
          
			FVector RebuiltScale = CurrentNetData.isActive ? FVector(1.0f) : FVector::ZeroVector;
			transform.SetScale3D(RebuiltScale);
          
			tempTransforms.Add(transform);
          
			index++;
		}
       
		MemberComponents[componentIndex]->BatchUpdateInstancesTransforms(0, tempTransforms, false, true, false);
	}
}

TArray<int32> AFluxPrimeCrowd::SplitDynamic(int32 Total, int32 NumGroups)
{
	TArray<int32> Results;
	int32 ActualGroups = FMath::Min(NumGroups, Total);

	if (ActualGroups <= 1) 
	{
		Results.Add(Total);
		return Results;
	}
    
	TSet<int32> UniqueCuts;
	while (UniqueCuts.Num() < ActualGroups - 1) 
	{
		UniqueCuts.Add(FMath::RandRange(1, Total - 1));
	}
    
	TArray<int32> Cuts = UniqueCuts.Array();
	Cuts.Sort();
    
	int32 CurrentPos = 0;
	for (int32 Cut : Cuts) 
	{
		Results.Add(Cut - CurrentPos);
		CurrentPos = Cut;
	}

	Results.Add(Total - CurrentPos);

	Results.Sort();
	return Results;
}

AFluxPrimeCrowd::AFluxPrimeCrowd()
{
	bReplicates = true;
	
	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	CapsuleComponent->SetCapsuleRadius(10, false);
	CapsuleComponent->SetCapsuleHalfHeight(5);
	RootComponent = CapsuleComponent;
}

void AFluxPrimeCrowd::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFluxPrimeCrowd, MemberTransforms);
}

void AFluxPrimeCrowd::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<int32> Splits;
	MemberTransforms.Reserve(MemberMeshes.Num());
	MemberTypeCounts.Reserve(MemberTransforms.Num());

	if (HasAuthority()) Splits = SplitDynamic(TotalMemebers, MemberMeshes.Num());
	
	for (int i = 0; i < MemberMeshes.Num(); i++)
	{
		FString name = FString::Printf(TEXT("CROWD_%d"), i);
		
		FAttachmentTransformRules AttachRules(
			EAttachmentRule::KeepRelative,
			EAttachmentRule::KeepRelative,
			EAttachmentRule::KeepRelative,
			false
		);
		UInstancedStaticMeshComponent* tempISMC = NewObject<UInstancedStaticMeshComponent>(this, FName(name));
		tempISMC->SetIsReplicated(true);
		tempISMC->SetStaticMesh(MemberMeshes[i]);
		tempISMC->AttachToComponent(RootComponent, AttachRules);
		tempISMC->RegisterComponent();
		tempISMC->ClearInstances();
		tempISMC->UpdateBounds();
		tempISMC->RecreatePhysicsState();
        
		MemberComponents.Add(tempISMC);
       
		if (!HasAuthority()) continue;
       
		MemberTypeCounts.Add(Splits[i]);
		
		float y = 300.0f * i;
       
		for (int32 j = 0; j < Splits[i]; j++)
		{
			float x = 100.0f * j;
          
			FTransform tempTransform;
			tempTransform.SetLocation(FVector(x, y, 0.0f));
        	
			int32 id = tempISMC->AddInstance(tempTransform, false);
        	
			FFluxPrimeNet tempNet;
			tempNet.NetLocation = tempTransform.GetLocation();
			tempNet.NetRotation = 0;
			tempNet.NetType = i;
			tempNet.NetID = id;
			tempNet.isActive = true;
            
			MemberTransforms.Add(tempNet);
		}
	}
}

void AFluxPrimeCrowd::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!HasAuthority()) return;
	
	AActor* target = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!target) return;

	FVector targetLocation = target->GetActorLocation();
	
	int32 index = 0; 
    
	for (int32 MeshIdx = 0; MeshIdx < MemberComponents.Num(); MeshIdx++)
	{
		TArray<FTransform> tempTransforms;
		int32 CountForThisMesh = MemberTypeCounts[MeshIdx];
       
		for (int32 j = 0; j < CountForThisMesh; j++)
		{
			FFluxPrimeNet& CurrentNetData = MemberTransforms[index];
           
			// move
			//CurrentNetData.NetLocation += FVector::ForwardVector * DeltaTime;
			
			FTransform transform;
			transform.SetLocation(CurrentNetData.NetLocation);
          
			FTransform componentTransform = MemberComponents[MeshIdx]->GetComponentTransform();
			FVector localTargetLoc = componentTransform.InverseTransformPosition(targetLocation);
       
			FRotator rot = UKismetMathLibrary::FindLookAtRotation(CurrentNetData.NetLocation, localTargetLoc);
			CurrentNetData.NetRotation = FRotator::CompressAxisToByte(rot.Yaw);
			FRotator Rot = FRotator(0.0f, rot.Yaw, 0.0f);
			transform.SetRotation(Rot.Quaternion());
          
			FVector RebuiltScale = CurrentNetData.isActive ? FVector(1.0f) : FVector::ZeroVector;
			transform.SetScale3D(RebuiltScale);
          
			tempTransforms.Add(transform);
          
			index++;
		}
       
		MemberComponents[MeshIdx]->BatchUpdateInstancesTransforms(0, tempTransforms, false, true, false);
	}
}
