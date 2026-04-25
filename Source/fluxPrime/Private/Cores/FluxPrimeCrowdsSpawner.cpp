// Fill out your copyright notice in the Description page of Project Settings.


#include "Cores/FluxPrimeCrowdsSpawner.h"

#include "EngineUtils.h"
#include "Crowds/FluxPrimeCrowdsManagerInterface.h"

void AFluxPrimeCrowdsSpawner::Spawning()
{
	if (CrowdsCatalog.IsEmpty()) return;
	
	FVector centerPoint = GetActorLocation();
	float randomAngleRadian = FMath::RandRange(0.0f, 2.0f * PI);

	float minSq = SpawnMinimalRadius * SpawnMinimalRadius;
	float maxSq = SpawnMaximalRadius * SpawnMaximalRadius;
	float randomRadius = FMath::Sqrt(FMath::RandRange(minSq, maxSq));

	float Sin, Cos;
	FMath::SinCos(&Sin, &Cos, randomAngleRadian);

	FVector targetLocation = centerPoint + FVector(Cos * randomRadius, Sin * randomRadius, 0.0f);

	float traceStartHeight = 5000.0f; 
	float traceEndHeight = -5000.0f;

	FVector TraceStart = FVector(targetLocation.X, targetLocation.Y, traceStartHeight);
	FVector TraceEnd = FVector(targetLocation.X, targetLocation.Y, traceEndHeight);
	
	FHitResult HitResult;
	FCollisionQueryParams TraceParams(FName(TEXT("SpawnGroundTrace")), true, this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_GroundHeight,
		TraceParams
	);

	targetLocation.Z = (bHit)? HitResult.ImpactPoint.Z : 0.0f;
	
	UObject* interfaceObject = CrowdsManagerInterface.GetObject();
	if (!interfaceObject) return;
	
	for (int i = 0; i < TotalSpawnCrowdPerSeconds; ++i)
	{
		int32 index = FMath::RandRange(0, CrowdsCatalog.Num() - 1);
		if (!CrowdsCatalog.IsValidIndex(index)) continue;
		
		IFluxPrimeCrowdsManagerInterface::Execute_SpawnCrowd(interfaceObject, CrowdsCatalog[i], targetLocation, FRotator::ZeroRotator);
	}
}

AFluxPrimeCrowdsSpawner::AFluxPrimeCrowdsSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFluxPrimeCrowdsSpawner::StartSpawning()
{
	if (!CrowdsManagerInterface) return;
	
	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle, 
		this, 
		&AFluxPrimeCrowdsSpawner::Spawning, 
		SpawnCrowdPerSeconds, 
		true);
}

void AFluxPrimeCrowdsSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	for (TActorIterator<AActor> temp(GetWorld()); temp; ++temp)
	{
		AActor* actor = *temp;
    
		if (actor && actor->Implements<UFluxPrimeCrowdsManagerInterface>())
		{
			CrowdsManagerInterface.SetObject(actor);
			CrowdsManagerInterface.SetInterface(Cast<IFluxPrimeCrowdsManagerInterface>(actor));
		}
	}
}

void AFluxPrimeCrowdsSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	
	Super::EndPlay(EndPlayReason);
}

void AFluxPrimeCrowdsSpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	FlushPersistentDebugLines(GetWorld());

	FVector centerLocation = GetActorLocation() + (FVector::UpVector * 1000);
	FVector yAxis = GetActorRightVector();
	FVector xAxis = GetActorForwardVector();

	DrawDebugCircle(
		GetWorld(), 
		centerLocation, 
		SpawnMinimalRadius, 
		32,
		FColor::Green, 
		true,
		-1.0f,
		0,
		12.0f,
		yAxis,
		xAxis,
		false
	);

	DrawDebugCircle(
		GetWorld(), 
		centerLocation, 
		SpawnMaximalRadius, 
		32, 
		FColor::Red, 
		true, 
		-1.0f, 
		0, 
		15.0f, 
		yAxis, 
		xAxis, 
		false
	);
}

#if WITH_EDITOR
void AFluxPrimeCrowdsSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AFluxPrimeCrowdsSpawner, SpawnMinimalRadius))
	{
		if (SpawnMinimalRadius > SpawnMaximalRadius)
			SpawnMinimalRadius = SpawnMaximalRadius;
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(AFluxPrimeCrowdsSpawner, SpawnMaximalRadius))
	{
		if (SpawnMaximalRadius < SpawnMinimalRadius)
			SpawnMaximalRadius = SpawnMinimalRadius;
	}
}
#endif
