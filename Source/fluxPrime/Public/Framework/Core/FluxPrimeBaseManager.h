// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FluxPrimeDataStore.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Framework/Data/FluxPrimeEntityCatalogIdentity.h"
#include "StructUtils/InstancedStruct.h"
#include "Framework/Data/FluxPrimeIdentityData.h"
#include "FluxPrimeBaseManager.generated.h"

class UManagerConfiguration;

//using FModuleExecute = void(*)(FInstancedStruct&, const float, FFluxPrimeDataStore&);
using FModuleExecute = void(*)(FInstancedStruct&, const float, TArray<FFluxPrimeArchetypeDataStore>&);

USTRUCT()
struct FModuleExecutor
{
	GENERATED_BODY()
	
	FInstancedStruct* Data = nullptr;
	FModuleExecute Execute = nullptr;
};

UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FLUXPRIME_API UFluxPrimeBaseManager : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TArray<FModuleExecutor> ModuleExecutors;
	
	UPROPERTY()
	TArray<FGameplayTag> StateID;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Entity Manager | CollisionProfile", meta = (AllowPrivateAccess = true))
	FName ProfileNameCollisionCrowds = "Pawn";
	
	UPROPERTY(EditAnywhere, Category = "Entity Manager | Configuration", meta = (AllowPrivateAccess = true))
	TObjectPtr<UManagerConfiguration> ManagerConfiguration;
	
	UPROPERTY(EditAnywhere, Category = "Entity Manager | Catalogs", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFluxPrimeEntityCatalogIdentity> EntityCatalogIdentity;
	
	TArray<FFluxPrimeArchetypeDataStore> DataStores;
	

private:
	template<typename T>
	static void ExecuteModule(FInstancedStruct& data, const float deltatime, TArray<FFluxPrimeArchetypeDataStore>& dataStore)
	{
		T& module = data.GetMutable<T>();
		module.ExecuteModule(deltatime, dataStore);
	}
	
public:
	UFluxPrimeBaseManager();

protected:
	template<typename T>
	int32 RegisterModule(FInstancedStruct& source)
	{
		FModuleExecutor executor;
		executor.Data = &source;
		executor.Execute = &ExecuteModule<T>;

		return ModuleExecutors.Add(MoveTemp(executor));
	}
	
	inline FModuleExecutor& GetModuleExecutor(uint32 index)
	{
		return ModuleExecutors[index];
	}
	
	void InitializedModule(uint32 count);
	void ExecuteModules(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores);
	
	inline void RegisterStateID(FGameplayTag identity)
	{
		if (GetStateID(identity) != INDEX_NONE) return;

		StateID.Add(identity);
	}
	
	inline int32 GetStateID(FGameplayTag identity) const
	{
		int32 index = 0;
		for (const auto& pair : StateID)
		{
			if (pair == identity) return index;
			index++;
		}

		return INDEX_NONE;
	}
};