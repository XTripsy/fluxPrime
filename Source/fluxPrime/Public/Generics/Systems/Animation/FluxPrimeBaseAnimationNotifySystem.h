#pragma once

#include "CoreMinimal.h"
#include "Framework/Systems/FluxPrimeBaseSystem.h"
#include "StructUtils/InstancedStruct.h"
#include "Framework/Core/FluxPrimeDataStore.h"
#include "Framework/Data/Wrapper/FluxPrimeWrapperInt.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "FluxPrimeBaseAnimationNotifySystem.generated.h"

using FNotifyExecute = void(*)(FInstancedStruct&, const int32, const int32, FFluxPrimeArchetypeDataStore&);
using FNotifyGetRequestID = int16(*)(const FInstancedStruct&, const int32);

USTRUCT()
struct FNotifyExecutor
{
	GENERATED_BODY()
	
	FInstancedStruct* Data = nullptr;
	FNotifyExecute Execute = nullptr;
	FNotifyGetRequestID GetRequestDataID = nullptr;
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

	template<typename T>
	static int16 GetRequestNotifyID(const FInstancedStruct& data, const int32 indexDataID)
	{
		const T& notify = data.Get<T>();
		// An unresolved notify ID can match INDEX_NONE in the legacy executor.
		if (!notify.CanSkipEmptyRequest() || !notify.EntityDataRequestNotifyID.IsValidIndex(indexDataID)) return INDEX_NONE;
		return notify.EntityDataRequestNotifyID[indexDataID];
	}
	
protected:
	template<typename T>
	int32 RegisterAbility(FInstancedStruct& source)
	{
		FNotifyExecutor executor;
		executor.Data = &source;
		executor.Execute = &ExecuteNotify<T>;
		executor.GetRequestDataID = &GetRequestNotifyID<T>;

		return NotifyExecutors.Add(MoveTemp(executor));
	}

	/**
	 * Resolves the shared request-notify buffer for one archetype when every executor points at the
	 * same tag. Returns nullptr when the executors disagree, so the caller falls back to the full scan.
	 * The request id is INDEX_NONE for entities with no pending notify.
	 */
	const FFluxPrimeWrapperInt32* ResolveSharedRequestNotify(FFluxPrimeArchetypeDataStore& dataStore, const int32 indexDataID) const
	{
		if (NotifyExecutors.IsEmpty()) return nullptr;

		int16 sharedRequestID = INDEX_NONE;
		for (const FNotifyExecutor& pair : NotifyExecutors)
		{
			if (!pair.Data || !pair.GetRequestDataID) return nullptr;

			const int16 requestID = pair.GetRequestDataID(*pair.Data, indexDataID);
			if (requestID == INDEX_NONE) return nullptr;

			if (sharedRequestID == INDEX_NONE)
			{
				sharedRequestID = requestID;
			}
			else if (sharedRequestID != requestID)
			{
				return nullptr;
			}
		}

		return reinterpret_cast<const FFluxPrimeWrapperInt32*>(dataStore.GetRawBufferData(sharedRequestID));
	}
	
public:
	void ExecuteSystem(float deltaTime, TArray<FFluxPrimeArchetypeDataStore>& dataStores)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FluxPrime_Animation_Notify_Systems);

		int16 indexDataID = 0;
		for (auto& dataStore : dataStores)
		{
			const int32 activeEntityCount = dataStore.GetActiveEntityCount();
			const FFluxPrimeWrapperInt32* sharedRequestNotify = ResolveSharedRequestNotify(dataStore, indexDataID);

			for (int indexEntity = 0; indexEntity < activeEntityCount; ++indexEntity)
			{
				if (sharedRequestNotify && sharedRequestNotify[indexEntity].ValueInt32 == INDEX_NONE) continue;

				for (auto& pair : NotifyExecutors)
				{
					pair.Execute(*pair.Data, indexDataID, indexEntity, dataStore);
				}
			}
			
			indexDataID++;
		}
	}
};