// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/Core/FluxPrimeBaseManager.h"

UFluxPrimeBaseManager::UFluxPrimeBaseManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFluxPrimeBaseManager::InitializedModule(uint32 count)
{
	ModuleExecutors.Reset();
	ModuleExecutors.Reserve(count);
}

void UFluxPrimeBaseManager::ExecuteModules(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
{
	if (!ModuleExecutors.IsEmpty())
	{
		for (auto& pair : ModuleExecutors)
		{
			pair.Execute(*pair.Data, deltaTime, dataStores);
		}
	}
}
