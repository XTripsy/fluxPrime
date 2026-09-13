#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "FluxPrimeBaseModule.generated.h"

using FSystemExecute = void(*)(FInstancedStruct&, const float, TArray<FFluxPrimeArchetypeDataStore>&);

struct FSystemExecutor
{
	FInstancedStruct* Data = nullptr;
	FSystemExecute Execute = nullptr;
};

USTRUCT()
struct FFluxPrimeBaseModule
{
	GENERATED_BODY()
	
public:
	virtual ~FFluxPrimeBaseModule() = default;
	
private:
	bool IsInitializedDone = false;
	TArray<FSystemExecutor> SystemExecutors;
	
private:
	template<typename T>
	static void ExecuteSystem(FInstancedStruct& data, const float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStore)
	{
		T& system = data.GetMutable<T>();

		system.ExecuteSystem(deltaTime, dataStore);
	}
	
protected:
	template<typename T>
	int32 RegisterSystem(FInstancedStruct& source)
	{
		FSystemExecutor executor;
		executor.Data = &source;
		executor.Execute = &ExecuteSystem<T>;

		return SystemExecutors.Add(MoveTemp(executor));
	}
	
	FSystemExecutor& GetSystemExecutor(uint32 index)
	{
		return SystemExecutors[index];
	}
	
	void StartInitializedSystem(uint32 count)
	{
		IsInitializedDone = false;
		SystemExecutors.Reset();
		SystemExecutors.Reserve(count);
	}
	
	void EndInitializedSystem()
	{
		IsInitializedDone = true;
	}
	
public:
	void ExecuteModule(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		if (!SystemExecutors.IsEmpty() && IsInitializedDone)
		{
			for (auto& pair : SystemExecutors)
			{
				pair.Execute(*pair.Data, deltaTime, dataStores);
			}
		}
	}
};
