#pragma once

#include "Framework/Core/FluxPrimeBaseManager.h"
#include "FluxPrimePauseTestManager.generated.h"

USTRUCT()
struct FFluxPrimePauseCounterModule
{
	GENERATED_BODY()
	int32 Count = 0;
	void ExecuteModule(float, TArray<FFluxPrimeArchetypeDataStore>&) { ++Count; }
};

UCLASS()
class UFluxPrimePauseTestManager : public UFluxPrimeBaseManager
{
	GENERATED_BODY()
public:
	void InstallCounter(FInstancedStruct& Storage, FModuleExecute Function)
	{
		InitializedModule(1);
		const int32 Index = RegisterModule<FFluxPrimePauseCounterModule>(Storage);
		GetModuleExecutor(Index).Execute = Function;
	}
	void Dispatch() { ExecuteModules(0.1f, DataStores); }
};
