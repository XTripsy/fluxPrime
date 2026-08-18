// Fill out your copyright notice in the Description page of Project Settings.


#include "Crowds/Components/FluxPrimeCrowdsManager.h"

#include "Crowds/Components/FluxPrimeTargetComponent.h"
#include "Engine/AssetManager.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

UFluxPrimeCrowdsManager::UFluxPrimeCrowdsManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false);
}

void UFluxPrimeCrowdsManager::InitializeManager(FFluxPrimeCrowdsManagerContext context)
{
	CrowdsComponents = context.CrowdsComponents;
}

void UFluxPrimeCrowdsManager::ShowDebug()
{
	FVector location = FVector::ZeroVector;
	FString debugData = "";
	
	for (int i = 0; i < CrowdsActive; ++i)
	{
		location = CrowdsDatas.CrowdsLocation[i] + (FVector::UpVector * FluxConfig::DebugLocationIdentity);
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

void UFluxPrimeCrowdsManager::PreLoading()
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
			FStreamableDelegate::CreateUObject(this, &UFluxPrimeCrowdsManager::Initialize)
		);
	}
	else Initialize();
}

void UFluxPrimeCrowdsManager::Initialize()
{
	CrowdsPool.Init(TArray<int16>(), CrowdsCatalog.Num());
	CrowdsHeadPool.Init(-1, CrowdsCatalog.Num());
	
	int8 index = 0;
	for (auto& temp : CrowdsCatalog)
	{
		CrowdsPool[index].Reserve(temp.CrowdsTotal);
		
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
		
		index++;
	}
	
	if (CrowdsMeshSoftRef.IsEmpty() || CrowdsAnimationSoftRef.IsEmpty()) return;
	
	InitializeComponentCrowds();
	StreamingHandle.Reset();
	
	// Crowds
	{
		FFluxPrimeCrowdsDataModuleContext context;
		context.crowdsComponents = CrowdsComponents;
		context.crowdsCatalog = &CrowdsCatalog;
		context.crowdsPool = &CrowdsPool;
		context.crowdsAnimationSoftRef = &CrowdsAnimationSoftRef;
		context.crowdsTotal = &CrowdsTotal;
		context.crowdsLookup = &CrowdsLookup;
		context.crowdsDatas = &CrowdsDatas;
		
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
		context.crowdsPool = &CrowdsPool;
		context.crowdsHeadPool = &CrowdsHeadPool;
		context.crowdsLookup = &CrowdsLookup;
		context.onCrowdsManagerActionChange = &OnCrowdsManagerActionChange;
		context.crowdsTarget = &CrowdsTarget;
		
		context.world = GetWorld();
		
		CrowdsSystemsModule.Initialize(context);
		CrowdsSystemsModule.InitializeSystems();
	}
	
	InitializedComponentSystems();
	
	FTimerHandle TimerHandle;

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			InitializedPlayer();
		},
		1.0f,
		false
	);
	//InitializedPlayer();
}

void UFluxPrimeCrowdsManager::InitializeComponentCrowds()
{
	for (int i = 0; i < CrowdsCatalog.Num(); ++i)
	{
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
	}
}

void UFluxPrimeCrowdsManager::InitializedComponentSystems()
{
	// Spawner
	{
		FFluxPrimeCrowdsSpawnerComponentContext context;
		context.crowdsActive = &CrowdsActive;
		context.crowdsTotal = &CrowdsTotal;
		context.crowdsPool = &CrowdsPool;
		context.crowdsHeadPool = &CrowdsHeadPool;
		context.crowdsData = &CrowdsDatas;
		context.crowdsTypes = &CrowdsTypes;
		context.crowdsLookup = &CrowdsLookup;
		context.world = GetWorld();
		
		CrowdsSpawnerModule.Initialize(context);
	}
	
}

void UFluxPrimeCrowdsManager::InitializedPlayer()
{
	AGameStateBase* gameState = GetWorld()->GetGameState<AGameStateBase>();

	TArray<APlayerState*> playerStates = gameState->PlayerArray;

	playerStates.Sort([](const APlayerState& A, const APlayerState& B)
	{
		return A.GetPlayerId() < B.GetPlayerId();
	});

	for (APlayerState* pair : playerStates)
	{
		if (!pair) continue;

		uint16 priorityPawn = 0;
		APawn* pawn = pair->GetPawn();
		if (UFluxPrimeTargetComponent* component = pawn->FindComponentByClass<UFluxPrimeTargetComponent>())
			priorityPawn = component->GetPriorityTarget();

		if (!pawn) continue;

		FFluxPrimeTargetCatalog catalog;
		catalog.CrowdsTarget = pawn;
		catalog.CrowdsTargetPriority = priorityPawn;
		
		CrowdsTarget.Add(catalog);
	}
}

void UFluxPrimeCrowdsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Manager);
	
	if (CrowdsComponents->IsEmpty() || CrowdsActive <= 0) return;
	
	CrowdsSystemsModule.TickSystems(DeltaTime);
	
	if (IsShowDebug) ShowDebug();
}

void UFluxPrimeCrowdsManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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

void UFluxPrimeCrowdsManager::OnActionChange(FInstancedStruct payload)
{
	if (payload.GetScriptStruct() == FFluxPrimeSpawnActionPayload::StaticStruct())
	{
		const FFluxPrimeSpawnActionPayload& data = payload.Get<FFluxPrimeSpawnActionPayload>();
		CrowdsSpawnerModule.SpawnCrowd(data.Identity, data.Location, data.Rotation);
	}
	
	if (payload.GetScriptStruct() == FFluxPrimeChangeTargetActionPayload::StaticStruct())
	{
		const FFluxPrimeChangeTargetActionPayload& data = payload.Get<FFluxPrimeChangeTargetActionPayload>();
		
		FFluxPrimeCrowdsLookup lookup;
		lookup.CrowdsID = data.CrowdID;
		lookup.CrowdsType = data.CrowdType;
		
		uint32 index = *CrowdsLookup.Find(lookup);
		
		if (index == INDEX_NONE) return;
		
		CrowdsDatas.CrowdsTargetID[index] = data.TargetID;
		CrowdsDatas.CrowdsTarget[index] = data.NewTargetLocation;
		CrowdsDatas.CrowdsRequestNeedReplan[index] = true;
	}
	
	if (payload.GetScriptStruct() == FFluxPrimeDamageActionPayload::StaticStruct())
	{
		const FFluxPrimeDamageActionPayload& data = payload.Get<FFluxPrimeDamageActionPayload>();
		
		FFluxPrimeCrowdsLookup lookup;
		lookup.CrowdsID = data.CrowdID;
		lookup.CrowdsType = data.CrowdType;
		
		uint32 index = *CrowdsLookup.Find(lookup);
		
		if (index == INDEX_NONE) return;
		
		CrowdsSystemsModule.Damage(index);
	}
}
