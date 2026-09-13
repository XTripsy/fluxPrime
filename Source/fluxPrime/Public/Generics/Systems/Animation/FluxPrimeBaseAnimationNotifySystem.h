#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "StructUtils/InstancedStruct.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#if IF_WITH_EDITOR
#include "ProfilingDebugging/CpuProfilerTrace.h"
#endif
#include "FluxPrimeBaseAnimationNotifySystem.generated.h"

using FNotifyExecute = void(*)(FInstancedStruct&, const int32, const int32, FFluxPrimeArchetypeDataStore&);

USTRUCT()
struct FNotifyExecutor
{
	GENERATED_BODY()
	
	FInstancedStruct* Data = nullptr;
	FNotifyExecute Execute = nullptr;
};

USTRUCT(BlueprintType)
struct FFluxPrimeBaseAnimationNotifySystem : public FFluxPrimeBaseSystem
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, meta = (BaseStruct="FluxPrimeNotifyBaseSystem", ExcludeBaseStruct, AllowPrivateAccess = true))
	TArray<FInstancedStruct> NotifyFragments;
	
	UPROPERTY()
	TArray<FNotifyExecutor> NotifyExecutors;
	
private:
	template<typename T>
	static void ExecuteNotify(FInstancedStruct& data, const int32 indexDataID, const int32 index, FFluxPrimeArchetypeDataStore& dataStore)
	{
		T& notify = data.GetMutable<T>();

		notify.ExecuteNotify(indexDataID, index, dataStore);
	}
	
protected:
	template<typename T>
	int32 RegisterAbility(FInstancedStruct& source)
	{
		FNotifyExecutor executor;
		executor.Data = &source;
		executor.Execute = &ExecuteNotify<T>;

		return NotifyExecutors.Add(MoveTemp(executor));
	}
	
public:
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
#if IF_WITH_EDITOR
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Notify_Systems);
#endif

		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			for (int indexEntity = 0; indexEntity < dataStore.GetActiveEntityCount(); ++indexEntity)
			{
				for (auto& pair : NotifyExecutors)
				{
					UE_LOG(LogTemp, Warning, TEXT("UPDATE NOTIFY SYSTEM"));
					pair.Execute(*pair.Data, indexDataID, indexEntity, dataStore);
				}
			}
			
			indexDataID++;
		}
	}
};