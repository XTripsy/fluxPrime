// Fill out your copyright notice in the Description page of Project Settings.


#include "Domains/Crowds/Managers/FluxPrimeCrowdsManager.h"
#include "fluxPrime.h"

#include "Engine/AssetManager.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetMathLibrary.h"
#include "Generics/Data/FluxPrimeAnimationIdentity.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsIdentity.h"
#include "Domains/Crowds/Configurations/ManagerConfiguration.h"
#include "Domains/Crowds/Managers/FluxPrimeCrowdsTargetManager.h"
#include "Domains/Crowds/Modules/FluxPrimeCrowdsDataModule.h"
#include "Domains/Crowds/Data/FluxPrimeCrowdsPayloadData.h"
#include "Domains/Crowds/Modules/FluxPrimeCrowdsSpawnerModule.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

UFluxPrimeCrowdsManager::UFluxPrimeCrowdsManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false);
}

bool UFluxPrimeCrowdsManager::ValidateAndResolveConfiguration()
{
	ResolvedCrowdIdentities.Reset();

	if (!ManagerConfiguration)
	{
		UE_LOG(LogFluxPrime, Error,
			TEXT("Crowds manager %s cannot initialize: ManagerConfiguration is not set."), *GetName());
		return false;
	}

	if (!EntityCatalogIdentity)
	{
		UE_LOG(LogFluxPrime, Error,
			TEXT("Crowds manager %s cannot initialize: EntityCatalogIdentity is not set."), *GetName());
		return false;
	}

	const TArray<FFluxPrimeArchetypeEntityCatalog>& catalog = EntityCatalogIdentity->ArchetypeEntityCatalog;

	if (catalog.IsEmpty())
	{
		UE_LOG(LogFluxPrime, Error,
			TEXT("Crowds manager %s cannot initialize: the entity catalog is empty."), *GetName());
		return false;
	}

	ResolvedCrowdIdentities.Reserve(catalog.Num());

	TSet<FName> resolvedNames;
	resolvedNames.Reserve(catalog.Num());

	for (int32 index = 0; index < catalog.Num(); ++index)
	{
		UFluxPrimeBaseIdentity* baseIdentity = catalog[index].EntityIdentity;
		UFluxPrimeCrowdsIdentity* identity = Cast<UFluxPrimeCrowdsIdentity>(baseIdentity);

		if (!identity)
		{
			UE_LOG(LogFluxPrime, Error,
				TEXT("Crowds manager %s cannot initialize: catalog index %d holds %s, expected UFluxPrimeCrowdsIdentity."),
				*GetName(), index, baseIdentity ? *baseIdentity->GetClass()->GetName() : TEXT("null"));
			ResolvedCrowdIdentities.Reset();
			return false;
		}

		if (identity->Identity.IsNone())
		{
			UE_LOG(LogFluxPrime, Error,
				TEXT("Crowds manager %s cannot initialize: catalog index %d (%s) has an empty Identity name."),
				*GetName(), index, *identity->GetName());
			ResolvedCrowdIdentities.Reset();
			return false;
		}

		if (resolvedNames.Contains(identity->Identity))
		{
			UE_LOG(LogFluxPrime, Error,
				TEXT("Crowds manager %s cannot initialize: catalog index %d repeats identity name '%s'."),
				*GetName(), index, *identity->Identity.ToString());
			ResolvedCrowdIdentities.Reset();
			return false;
		}

		if (!identity->StateIdentity)
		{
			UE_LOG(LogFluxPrime, Error,
				TEXT("Crowds manager %s cannot initialize: identity '%s' (catalog index %d) has no StateIdentity."),
				*GetName(), *identity->Identity.ToString(), index);
			ResolvedCrowdIdentities.Reset();
			return false;
		}

		resolvedNames.Add(identity->Identity);
		ResolvedCrowdIdentities.Add(identity);
	}

	return true;
}

void UFluxPrimeCrowdsManager::PreLoading()
{
	if (!ValidateAndResolveConfiguration()) return;

#if WITH_EDITOR
	
	{
		TObjectPtr<UManagerConfiguration> LocalManagerConfiguration = DuplicateObject<UManagerConfiguration>(ManagerConfiguration, this);
		ManagerConfiguration = LocalManagerConfiguration;
	}
	
#endif
	
	ModuleFragments = &ManagerConfiguration->ModuleFragments;
	
	TArray<FSoftObjectPath> pathsToLoad;

	for (UFluxPrimeCrowdsIdentity* identity : ResolvedCrowdIdentities)
	{
		if (identity->Mesh.IsPending()) pathsToLoad.Add(identity->Mesh.ToSoftObjectPath());
   
		if (identity->AnimationData.IsPending()) pathsToLoad.Add(identity->AnimationData.ToSoftObjectPath());
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
	const int32 entityCatalogCount = ResolvedCrowdIdentities.Num();

	CrowdsMeshSoftRef.Reset();
	CrowdsAnimationSoftRef.Reset();

	TArray<FString> missingAssets;

	for (int i = 0; i < entityCatalogCount; ++i)
	{
		UFluxPrimeCrowdsIdentity* identity = ResolvedCrowdIdentities[i];
		
		if (UStaticMesh* meshCached = identity->Mesh.Get())
		{
			CrowdsMeshSoftRef.Add(identity->Identity, meshCached);
			UE_LOG(LogFluxPrime, Verbose, TEXT("Loaded mesh for archetype %s"), *identity->Identity.ToString());
		}
		else
		{
			missingAssets.Add(FString::Printf(TEXT("mesh '%s'"), *identity->Identity.ToString()));
		}

		if (UFluxPrimeAnimationData* animationCached = identity->AnimationData.Get())
		{
			CrowdsAnimationSoftRef.Add(identity->Identity, animationCached);
			UE_LOG(LogFluxPrime, Verbose, TEXT("Loaded animation for archetype %s"), *identity->Identity.ToString());
		}
		else
		{
			missingAssets.Add(FString::Printf(TEXT("animation '%s'"), *identity->Identity.ToString()));
		}
	}
	
	if (!missingAssets.IsEmpty())
	{
		UE_LOG(LogFluxPrime, Error,
			TEXT("Crowds manager %s cannot initialize: missing assets for %s. ")
			TEXT("Check that every UFluxPrimeCrowdsIdentity entry references loadable mesh and animation assets."),
			*GetName(), *FString::Join(missingAssets, TEXT(", ")));
		return;
	}
	
	InitializeComponentCrowds();
	StreamingHandle.Reset();

	ResolvedAnimationData.Reset();
	ResolvedAnimationData.Reserve(entityCatalogCount);

	for (int i = 0; i < entityCatalogCount; ++i)
	{
		UFluxPrimeCrowdsIdentity* identity = ResolvedCrowdIdentities[i];
		ResolvedAnimationData.Add(CrowdsAnimationSoftRef.FindChecked(identity->Identity).Get());
	}
	
	for (int i = 0; i < entityCatalogCount; ++i)
	{
		UFluxPrimeCrowdsIdentity* identity = ResolvedCrowdIdentities[i];
		
		for (auto& pairState : identity->StateIdentity->StateData)
		{
			RegisterStateID(pairState.IdentityState);
		}
		
		if (UFluxPrimeAnimationData* dataAnim = CrowdsAnimationSoftRef.FindChecked(identity->Identity).Get())
		{
			for (auto& pairAnimation : dataAnim->DataAnimations)
			{
				for (auto& pairNotify : pairAnimation.AnimationNotifies)
				{
					RegisterNotifyID(pairNotify.AnimationNotifyTag);
				}
			}
		}
	}
	
	InitializedModules();
}

void UFluxPrimeCrowdsManager::InitializedModules()
{
	if (!ModuleFragments)
	{
		UE_LOG(LogFluxPrime, Error, TEXT("Crowds manager %s cannot initialize modules: no module configuration."), *GetName());
		return;
	}

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
			context.hasAuthority = GetOwner()->HasAuthority();
		
			FModuleExecutor& Executor = GetModuleExecutor(executorIndex);
			FFluxPrimeCrowdsDataModule& module = Executor.Data->GetMutable<FFluxPrimeCrowdsDataModule>();
			module.Initialized(context);
			if (!module.IsAllDataValid) return;
			continue;
		}
		
		if (fragment.GetPtr<FFluxPrimeCrowdsSystemsModule>())
		{
			CrowdsSystemModuleIndex = i;
			const int32 executorIndex = RegisterModule<FFluxPrimeCrowdsSystemsModule>(fragment);
			
			FFluxPrimeCrowdsSystemsModuleContext context;
		
			context.world = GetWorld();
			context.crowdsComponents = &CrowdsComponents;
			context.entityCatalog = &EntityCatalogIdentity->ArchetypeEntityCatalog;
			context.resolvedIdentities = &ResolvedCrowdIdentities;
			context.resolvedAnimationData = &ResolvedAnimationData;
			context.hasAuthority = GetOwner()->HasAuthority();
			context.globalStateCount = GetStateCount();
			
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
			module.Initialized(DataStores, GetOwner()->HasAuthority());
			continue;
		}
	}

	if (CrowdsDataModuleIndex == INDEX_NONE)
	{
		UE_LOG(LogFluxPrime, Error, TEXT("Crowds manager %s has no data module; no buffers will be registered."), *GetName());
	}

	if (CrowdsSystemModuleIndex == INDEX_NONE)
	{
		UE_LOG(LogFluxPrime, Error, TEXT("Crowds manager %s has no systems module; no systems will run."), *GetName());
	}

	if (CrowdsSpawnerModuleIndex == INDEX_NONE)
	{
		UE_LOG(LogFluxPrime, Error, TEXT("Crowds manager %s has no spawner module; spawn requests will be ignored."), *GetName());
	}
}

void UFluxPrimeCrowdsManager::InitializeComponentCrowds()
{	
	for (int i = 0; i < ResolvedCrowdIdentities.Num(); ++i)
	{
		UFluxPrimeCrowdsIdentity* identity = ResolvedCrowdIdentities[i];
		if (!identity) continue;

		const TSoftObjectPtr<UStaticMesh>* foundMesh = CrowdsMeshSoftRef.Find(identity->Identity);
		if (!foundMesh || !foundMesh->IsValid())
		{
			UE_LOG(LogFluxPrime, Error,
				TEXT("Crowds manager %s skipped archetype '%s': no loaded static mesh."),
				*GetName(), *identity->Identity.ToString());
			continue;
		}

		FName name = identity->Identity;
    		
		FAttachmentTransformRules AttachRules(
			EAttachmentRule::KeepRelative,
			EAttachmentRule::KeepRelative,
			EAttachmentRule::KeepRelative,
			false
		);
		
		UInstancedStaticMeshComponent* tempISMC = NewObject<UInstancedStaticMeshComponent>(GetOwner(), name);
		tempISMC->SetIsReplicated(false);
		tempISMC->SetStaticMesh(foundMesh->Get());
		tempISMC->AttachToComponent(GetOwner()->GetRootComponent(), AttachRules);
		tempISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		tempISMC->ClearInstances();
		tempISMC->UpdateBounds();
		tempISMC->NumCustomDataFloats = 6;
		tempISMC->bReceivesDecals = false;
		tempISMC->CastShadow = false;
		tempISMC->RegisterComponent();
		tempISMC->SetHiddenInGame(false);
		CrowdsComponents.Add(tempISMC);
		
		CrowdsTypes.Add(identity->Identity, i);
		UE_LOG(LogFluxPrime, Verbose, TEXT("Registered archetype %s at index %d"), *identity->Identity.ToString(), i);
	}
}

void UFluxPrimeCrowdsManager::InitializedPlayer()
{
	UWorld* world = GetWorld();
	if (!world)
	{
		UE_LOG(LogFluxPrime, Error,
			TEXT("Crowds manager %s cannot register targets: no world."), *GetName());
		return;
	}

	AGameStateBase* gameState = world->GetGameState<AGameStateBase>();
	if (!gameState)
	{
		UE_LOG(LogFluxPrime, Warning,
			TEXT("Crowds manager %s cannot register targets yet: no game state. Call InitializedPlayer after the game state exists."),
			*GetName());
		return;
	}

	TArray<APlayerState*> playerStates = gameState->PlayerArray;

	playerStates.Sort([](const APlayerState& A, const APlayerState& B)
	{
		return A.GetPlayerId() < B.GetPlayerId();
	});
	
	CrowdsTarget.Reset();

	for (APlayerState* pair : playerStates)
	{
		UE_LOG(LogFluxPrime, VeryVerbose, TEXT("Evaluating player state for target registration"));
		if (!IsValid(pair)) continue;

		APawn* pawn = pair->GetPawn();
		if (!pawn) continue;

		uint16 priorityPawn = 0;
		if (UFluxPrimeCrowdsTargetManager* component = pawn->FindComponentByClass<UFluxPrimeCrowdsTargetManager>())
			priorityPawn = component->GetPriorityTarget();
		
		UE_LOG(LogFluxPrime, VeryVerbose, TEXT("Resolved target priority %u"), priorityPawn);
		
		FUniqueNetIdRepl id = pair->GetUniqueId();

		FFluxPrimeTargetCatalog catalog;
		catalog.CrowdsTarget = pawn;
		catalog.CrowdsTargetID = id;
		catalog.CrowdsTargetPriority = priorityPawn;
		
		CrowdsTarget.Add(catalog);
		PlayerKillsData.Add(id, 0);
		
		UE_LOG(LogFluxPrime, Verbose, TEXT("Registered player target id %s"), *id.ToString());
		UE_LOG(LogFluxPrime, VeryVerbose, TEXT("Added target catalog entry for %s"), *id.ToString());
	}
}

void UFluxPrimeCrowdsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Manager);
	
	if (CrowdsComponents.IsEmpty()) return;
	
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

	for (auto& data : DataStores)
	{
		data.FreeManualBuffer();
	}
	
	Super::EndPlay(EndPlayReason);
}

void UFluxPrimeCrowdsManager::RequestAction(const FInstancedStruct& payload)
{
	if (payload.GetScriptStruct() == FFluxPrimeCrowdsSpawnPayload::StaticStruct())
	{
		const FFluxPrimeCrowdsSpawnPayload* pair = payload.GetPtr<FFluxPrimeCrowdsSpawnPayload>();
		if (!pair) return;

		FFluxPrimeCrowdsSpawnerModule* spawnerModule = FindModule<FFluxPrimeCrowdsSpawnerModule>(CrowdsSpawnerModuleIndex);
		if (!spawnerModule) return;

		FFluxPrimeCrowdsSpawnerComponentContext context;
		context.crowdsTypes = &CrowdsTypes;
		context.dataStores = &DataStores;
		context.identity = pair->Identity;
		context.rotation = pair->Rotation;
		context.newState = pair->NewState;
		context.getStateID = [this](FGameplayTag Identity)
		{
			return GetStateID(Identity);
		};

		TArray<FVector_NetQuantize10> listSpawnLocation;
		listSpawnLocation.Reserve(pair->TotalEntityToSpawn);
	
		for (int i = 0; i < pair->TotalEntityToSpawn; ++i)
		{
			FVector resultLocation = UKismetMathLibrary::RandomPointInBoundingBox(pair->Location, pair->RandomBoundingBox);
			listSpawnLocation.Add(resultLocation);
		
			context.location = resultLocation;
			spawnerModule->SpawnCrowd(context);
		}
	
		FFluxPrimeCrwodsSpawnActionPayload data;
		data.Identity = pair->Identity;
		data.Locations = MoveTemp(listSpawnLocation);
		data.Rotation = pair->Rotation;
		data.NewState = pair->NewState;
	
		OnCrowdsManagerActionChange.ExecuteIfBound(FInstancedStruct::Make(data));
	}
	
	if (payload.GetScriptStruct() == FFluxPrimeCrowdsShootRayPayload::StaticStruct())
	{
		const FFluxPrimeCrowdsShootRayPayload* pair = payload.GetPtr<FFluxPrimeCrowdsShootRayPayload>();
		if (!pair) return;

		FFluxPrimeCrowdsSystemsModule* systemsModule = FindModule<FFluxPrimeCrowdsSystemsModule>(CrowdsSystemModuleIndex);
		if (!systemsModule) return;

		auto* raycastSystem = systemsModule->GetSystem<FFluxPrimeRaycastSystem>();
		auto* healthSystem = systemsModule->GetSystem<FFluxPrimeHealthSystem>();
		if (!raycastSystem || !healthSystem) return;
		
		int32 entityType = INDEX_NONE;
		int32 entityIndex = INDEX_NONE;
		
		raycastSystem->ShootRay(DataStores, pair->StartRayLocation, pair->EndRayLocation, entityType, entityIndex);
		if (entityType == INDEX_NONE || entityIndex == INDEX_NONE) return;

		healthSystem->TakeDamage(DataStores[entityType], entityType, entityIndex, pair->CrowdDamageTaken);

		auto* killEventSystem = systemsModule->GetSystem<FFluxPrimeCrowdsKillEventSystem>();
		if (!killEventSystem) return;
		
		if (killEventSystem->CekKillEvent(DataStores[entityType], entityType, entityIndex, pair->CrowdDamageTaken))
		{
			int32& valueKill = PlayerKillsData.FindOrAdd(pair->UniqueId);
			valueKill++;

			FFluxPrimeCrowdsKillDataPayload data;
			data.PlayerId = pair->UniqueId;
			data.PlayerKillCount = valueKill;
            
			OnCrowdsManagerActionChange.ExecuteIfBound(FInstancedStruct::Make(data));
		}
	}
	
	if (payload.GetScriptStruct() == FFluxPrimeCrwodsUpdatePriorityActionPayload::StaticStruct())
	{
		const FFluxPrimeCrwodsUpdatePriorityActionPayload* pair = payload.GetPtr<FFluxPrimeCrwodsUpdatePriorityActionPayload>();
		if (!pair) return;

		for (auto& pairTarget : CrowdsTarget)
		{
			if (pairTarget.CrowdsTargetID != pair->PlayerID) continue;
			
			pairTarget.CrowdsTargetPriority = pair->NewPriority;
		}
	}
}

void UFluxPrimeCrowdsManager::OnActionChange(const FInstancedStruct& payload)
{
	if (payload.GetScriptStruct() == FFluxPrimeCrwodsSpawnActionPayload::StaticStruct())
	{
		FFluxPrimeCrowdsSpawnerModule* spawnerModule = FindModule<FFluxPrimeCrowdsSpawnerModule>(CrowdsSpawnerModuleIndex);
		if (!spawnerModule) return;

		const FFluxPrimeCrwodsSpawnActionPayload& data = payload.Get<FFluxPrimeCrwodsSpawnActionPayload>();

		FFluxPrimeCrowdsSpawnerComponentContext context;
		context.crowdsTypes = &CrowdsTypes;
		context.dataStores = &DataStores;
		context.identity = data.Identity;
		context.rotation = data.Rotation;
		context.newState = data.NewState;
		context.getStateID = [this](FGameplayTag Identity)
		{
			return GetStateID(Identity);
		};
		
		for (auto& pairLocation : data.Locations)
		{
			context.location = pairLocation;
			spawnerModule->SpawnNetCrowd(context);
		}
	}
	
	if (payload.GetScriptStruct() == FFluxPrimeCrowdsSnapshotDataPayload::StaticStruct() ||
		payload.GetScriptStruct() == FFluxPrimeCrowdsSnapshotActiveEntityPayload::StaticStruct())
	{
		FFluxPrimeCrowdsSystemsModule* systemsModule = FindModule<FFluxPrimeCrowdsSystemsModule>(CrowdsSystemModuleIndex);
		if (!systemsModule) return;

		auto* snapshotSystem = systemsModule->GetSystem<FFluxPrimeCrowdsSnapshotSystem>();
		if (!snapshotSystem) return;
		
		snapshotSystem->SyncData(DataStores, payload);
	}
	
	if (payload.GetScriptStruct() == FFluxPrimeCrowdsKillDataPayload::StaticStruct())
	{
		const FFluxPrimeCrowdsKillDataPayload& data = payload.Get<FFluxPrimeCrowdsKillDataPayload>();
		
		PlayerKillsData.Add(data.PlayerId, data.PlayerKillCount);
	}
}
