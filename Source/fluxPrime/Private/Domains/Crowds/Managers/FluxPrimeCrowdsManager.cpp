// Fill out your copyright notice in the Description page of Project Settings.


#include "Domains/Crowds/Managers/FluxPrimeCrowdsManager.h"

#include "Generics/Data/FluxPrimeAnimationIdentity.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsIdentity.h"
#include "Domains/Crowds/Managers/ManagerConfiguration/ManagerConfiguration.h"
#include "Domains/Crowds/Managers/FluxPrimeCrowdsTargetManager.h"
#include "Domains/Crowds/Modules/FluxPrimeCrowdsDataModule.h"
#include "Engine/AssetManager.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#if IF_WITH_EDITOR
#include "ProfilingDebugging/CpuProfilerTrace.h"
#endif

UFluxPrimeCrowdsManager::UFluxPrimeCrowdsManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false);
}

void UFluxPrimeCrowdsManager::PreLoading()
{
	ModuleFragments = &ManagerConfiguration->ModuleFragments;
	
	TArray<FSoftObjectPath> pathsToLoad;

	for (int i = 0; i < EntityCatalogIdentity->ArchetypeEntityCatalog.Num(); ++i)
	{
		UFluxPrimeCrowdsIdentity* identity = Cast<UFluxPrimeCrowdsIdentity>(EntityCatalogIdentity->ArchetypeEntityCatalog[i].EntityIdentity);
		
		if (!identity->Mesh.IsPending()) continue;
		pathsToLoad.Add(identity->Mesh.ToSoftObjectPath());
		
		if (!identity->AnimationData.IsPending()) continue;
		pathsToLoad.Add(identity->AnimationData.ToSoftObjectPath());
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
	int32 entityCatalogCount = EntityCatalogIdentity->ArchetypeEntityCatalog.Num(); 
	
	for (int i = 0; i < entityCatalogCount; ++i)
	{
		UFluxPrimeCrowdsIdentity* identity = Cast<UFluxPrimeCrowdsIdentity>(EntityCatalogIdentity->ArchetypeEntityCatalog[i].EntityIdentity);
		
		if (UStaticMesh* meshCached =identity->Mesh.Get())
		{
			CrowdsMeshSoftRef.Add(identity->Identity, meshCached);
			UE_LOG(LogTemp, Error, TEXT("SUCESS MESH LOAD"));
		}

		if (UFluxPrimeAnimationData* animationCached = identity->AnimationData.Get())
		{
			CrowdsAnimationSoftRef.Add(identity->Identity, animationCached);
			UE_LOG(LogTemp, Error, TEXT("SUCESS ANIM LOAD"));
		}
	}
	
	if (CrowdsMeshSoftRef.IsEmpty() || CrowdsAnimationSoftRef.IsEmpty()) return;
	
	InitializeComponentCrowds();
	StreamingHandle.Reset();
	
	for (int i = 0; i < entityCatalogCount; ++i)
	{
		UFluxPrimeCrowdsIdentity* identity = Cast<UFluxPrimeCrowdsIdentity>(EntityCatalogIdentity->ArchetypeEntityCatalog[i].EntityIdentity);
		
		for (auto& pairState : identity->StateIdentity->StateData)
		{
			RegisterStateID(pairState.IdentityState);
		}
		
		UFluxPrimeAnimationData* dataAnim = CrowdsAnimationSoftRef[identity->Identity].Get();

		for (auto& pairAnimation : dataAnim->DataAnimations)
		{
			for (auto& pairNotify : pairAnimation.AnimationNotifies)
			{
				RegisterNotifyID(pairNotify.AnimationNotifyTag);
			}
		}
	}
	
	InitializedModules();
	
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
}

void UFluxPrimeCrowdsManager::InitializedModules()
{
	auto& moduleFragments = *ModuleFragments;
	
	InitializedModule(moduleFragments.Num());

	for (int i = 0; i < moduleFragments.Num(); ++i)
	{
		FInstancedStruct& fragment = moduleFragments[i];
		
		if (fragment.GetPtr<FFluxPrimeCrowdsDataModule>())
		{
			CrowdsDataModuleIndex = i;
			const int32 executorIndex = RegisterModule<FFluxPrimeCrowdsDataModule>(fragment);
			
			FFluxPrimeCrowdsDataModuleContext context;
			context.crowdsComponents = &CrowdsComponents;
			context.entityCatalog = &EntityCatalogIdentity->ArchetypeEntityCatalog;
			context.dataStores = &DataStores;
		
			FModuleExecutor& Executor = GetModuleExecutor(executorIndex);
			FFluxPrimeCrowdsDataModule& module = Executor.Data->GetMutable<FFluxPrimeCrowdsDataModule>();
			module.NewInitialized(context);
			continue;
		}
		
		if (fragment.GetPtr<FFluxPrimeCrowdsSystemsModule>())
		{
			CrowdsSystemModuleIndex = i;
			const int32 executorIndex = RegisterModule<FFluxPrimeCrowdsSystemsModule>(fragment);
			
			FFluxPrimeCrowdsSystemsModuleContext context;
		
			context.world = GetWorld();
			context.crowdsComponents = &CrowdsComponents;
			context.entityCatalog = &EntityCatalogIdentity->ArchetypeEntityCatalog;;
			context.crowdsAnimationSoftRef = &CrowdsAnimationSoftRef;
			
			context.onCrowdsManagerActionChange = &OnCrowdsManagerActionChange;
			context.crowdsTarget = &CrowdsTarget;
			context.getStateID = [this](FGameplayTag Identity)
			{
				return GetStateID(Identity);
			};
			context.getNotifyID = [this](FGameplayTag Identity)
			{
				return GetNotifyID(Identity);
			};
		
			context.dataStores = &DataStores;
		
			FModuleExecutor& Executor = GetModuleExecutor(executorIndex);
			FFluxPrimeCrowdsSystemsModule& module = Executor.Data->GetMutable<FFluxPrimeCrowdsSystemsModule>();
			module.InitializeSystems(context);
			continue;
		}
		
		if (fragment.GetPtr<FFluxPrimeCrowdsSpawnerModule>())
		{
			CrowdsSpawnerModuleIndex = i;
			const int32 executorIndex = RegisterModule<FFluxPrimeCrowdsSpawnerModule>(fragment);
			
			FModuleExecutor& Executor = GetModuleExecutor(executorIndex);
			FFluxPrimeCrowdsSpawnerModule& module = Executor.Data->GetMutable<FFluxPrimeCrowdsSpawnerModule>();
			module.Initialized(DataStores);
			continue;
		}
	}
}

void UFluxPrimeCrowdsManager::InitializeComponentCrowds()
{	
	for (int i = 0; i < EntityCatalogIdentity->ArchetypeEntityCatalog.Num(); ++i)
	{
		UFluxPrimeCrowdsIdentity* identity = Cast<UFluxPrimeCrowdsIdentity>(EntityCatalogIdentity->ArchetypeEntityCatalog[i].EntityIdentity);
		FName name = identity->Identity;
    		
		FAttachmentTransformRules AttachRules(
			EAttachmentRule::KeepRelative,
			EAttachmentRule::KeepRelative,
			EAttachmentRule::KeepRelative,
			false
		);
		
		UInstancedStaticMeshComponent* tempISMC = NewObject<UInstancedStaticMeshComponent>(GetOwner(), name);
		tempISMC->SetIsReplicated(false);
		tempISMC->SetStaticMesh(CrowdsMeshSoftRef[identity->Identity].Get());
		tempISMC->AttachToComponent(GetOwner()->GetRootComponent(), AttachRules);
		tempISMC->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		tempISMC->SetCollisionProfileName(ProfileNameCollisionCrowds);
		tempISMC->ClearInstances();
		tempISMC->UpdateBounds();
		tempISMC->RecreatePhysicsState();
		tempISMC->NumCustomDataFloats = 6;
		tempISMC->bReceivesDecals = false;
		tempISMC->RegisterComponent();
		CrowdsComponents.Add(tempISMC);
		
		CrowdsTypes.Add(identity->Identity, i);
		UE_LOG(LogTemp, Log, TEXT("ENTITY:: Name %s | Type %d"), *identity->Identity.ToString(), i);
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
		if (UFluxPrimeCrowdsTargetManager* component = pawn->FindComponentByClass<UFluxPrimeCrowdsTargetManager>())
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
	
#if IF_WITH_EDITOR
	TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Manager);
#endif
	
	if (CrowdsComponents.IsEmpty()) return;

	int16 indexData = 0;
	for (auto& dataStore : DataStores)
	{
		UE_LOG(LogTemp, Log, TEXT("ACTIVE %d | IndexData %d"), dataStore.GetActiveEntityCount(), indexData);
		indexData++;
	}
	
	ExecuteModules(DeltaTime, DataStores);
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
	OnCrowdsManagerActionChange.Unbind();
	
	Super::EndPlay(EndPlayReason);
}

void UFluxPrimeCrowdsManager::OnActionChange(FInstancedStruct payload)
{
	auto& moduleFragments = *ModuleFragments;
	
	if (payload.GetScriptStruct() == FFluxPrimeSpawnActionPayload::StaticStruct())
	{
		if (!ModuleFragments || CrowdsSpawnerModuleIndex == INDEX_NONE) return;
		const FFluxPrimeSpawnActionPayload& data = payload.Get<FFluxPrimeSpawnActionPayload>();
		
		FFluxPrimeCrowdsSpawnerComponentContext context;
		context.crowdsTypes = &CrowdsTypes;
		context.dataStores = &DataStores;
		context.identity = data.Identity;
		context.location = data.Location;
		context.rotation = data.Rotation;
		context.newState = data.NewState;
		context.getStateID = [this](FGameplayTag Identity)
		{
			return GetStateID(Identity);
		};
		moduleFragments[CrowdsSpawnerModuleIndex].GetMutable<FFluxPrimeCrowdsSpawnerModule>().SpawnCrowd(context);
	}
	
	if (payload.GetScriptStruct() == FFluxPrimeChangeTargetActionPayload::StaticStruct())
	{
		const FFluxPrimeChangeTargetActionPayload& data = payload.Get<FFluxPrimeChangeTargetActionPayload>();
		
		int32 typeIndex = data.CrowdType;
		int32 indexData = DataStores[typeIndex].GetIndexDataLookUp(data.CrowdID);
		
		if (indexData == INDEX_NONE) return;
		
		moduleFragments[CrowdsSystemModuleIndex].GetMutable<FFluxPrimeCrowdsSystemsModule>().SyncNewTarget(DataStores[typeIndex], typeIndex, indexData, data.TargetID, data.NewTargetLocation, true);
	}
	
	if (payload.GetScriptStruct() == FFluxPrimeDamageActionPayload::StaticStruct())
	{
		const FFluxPrimeDamageActionPayload& data = payload.Get<FFluxPrimeDamageActionPayload>();
		
		int32 typeIndex = data.CrowdType;
		int32 indexData = DataStores[typeIndex].GetIndexDataLookUp(data.CrowdID);
		
		if (indexData == INDEX_NONE) return;
		
		moduleFragments[CrowdsSpawnerModuleIndex].GetMutable<FFluxPrimeCrowdsSystemsModule>().TakeDamage(DataStores[typeIndex], typeIndex, indexData, data.CrowdDamageTaken);
	}
}