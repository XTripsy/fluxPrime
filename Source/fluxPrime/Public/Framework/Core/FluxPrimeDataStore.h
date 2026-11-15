#pragma once

#include "CoreMinimal.h"
#include "fluxPrime.h"
#include "GameplayTagContainer.h"

struct FFluxPrimeArchetypeDataStore
{
private:
	int32 ActiveEntityCount = 0;
	int32 TotalEntityCount = 0;
	TArray<UScriptStruct*> RawStructTypeDatas;
	TArray<uint8*> RawDataBuffers;
	TArray<int32> RawElementSizes;
	TArray<int32> RawElementCounts;
	TArray<FGameplayTag> DataID;

	void MoveFrom(FFluxPrimeArchetypeDataStore& other) noexcept
	{
		DataID = MoveTemp(other.DataID);
		RawStructTypeDatas = MoveTemp(other.RawStructTypeDatas);
		RawDataBuffers = MoveTemp(other.RawDataBuffers);
		RawElementSizes = MoveTemp(other.RawElementSizes);
		RawElementCounts = MoveTemp(other.RawElementCounts);
		ActiveEntityCount = other.ActiveEntityCount;
		TotalEntityCount = other.TotalEntityCount;

		other.ActiveEntityCount = 0;
		other.TotalEntityCount = 0;
	}
	
public:
	FFluxPrimeArchetypeDataStore() = default;
	~FFluxPrimeArchetypeDataStore()
	{
		FreeManualBuffer();
	}

	FFluxPrimeArchetypeDataStore(const FFluxPrimeArchetypeDataStore&) = delete;
	FFluxPrimeArchetypeDataStore& operator=(const FFluxPrimeArchetypeDataStore&) = delete;

	FFluxPrimeArchetypeDataStore(FFluxPrimeArchetypeDataStore&& other) noexcept
	{
		MoveFrom(other);
	}

	FFluxPrimeArchetypeDataStore& operator=(FFluxPrimeArchetypeDataStore&& other) noexcept
	{
		if (this == &other) return *this;

		FreeManualBuffer();
		MoveFrom(other);
		return *this;
	}
	
	void FreeManualBuffer()
	{
		for (int32 index = 0; index < RawDataBuffers.Num(); ++index)
		{
			uint8* buffer = RawDataBuffers[index];
			if (!buffer) continue;

			if (RawStructTypeDatas.IsValidIndex(index) && RawElementSizes.IsValidIndex(index) &&
				RawElementCounts.IsValidIndex(index))
			{
				UScriptStruct* structType = RawStructTypeDatas[index];
				const int32 elementSize = RawElementSizes[index];
				const int32 elementCount = RawElementCounts[index];

				if (structType && elementSize > 0)
				{
					for (int32 element = 0; element < elementCount; ++element)
					{
						structType->DestroyStruct(buffer + (element * elementSize));
					}
				}
			}

			FMemory::Free(buffer);
		}
		
		RawStructTypeDatas.Empty();
		RawDataBuffers.Empty();
		RawElementSizes.Empty();
		RawElementCounts.Empty();
		DataID.Empty();
		ActiveEntityCount = 0;
		TotalEntityCount = 0;
	}
	
	/**
	 * Registers one tag and its buffer in a single step. The three parallel arrays are appended
	 * together, so a duplicate tag is rejected before any memory is allocated.
	 */
	bool AddBuffer(const FGameplayTag tag, UScriptStruct* structType, const int32 elementSize, const int32 count)
	{
		if (structType == nullptr || elementSize <= 0 || count < 0) return false;
		if (GetDataID(tag) != INDEX_NONE) return false;

		uint8* allocatedMemory = static_cast<uint8*>(FMemory::Malloc(elementSize * count));
		if (allocatedMemory == nullptr) return false;

		for (int32 i = 0; i < count; ++i)
		{
			structType->InitializeStruct(allocatedMemory + (i * elementSize));
		}

		DataID.Add(tag);
		RawStructTypeDatas.Add(structType);
		RawDataBuffers.Add(allocatedMemory);
		RawElementSizes.Add(elementSize);
		RawElementCounts.Add(count);
		return true;
	}
	
	bool TryResolveTag(const FGameplayTag tag, const UScriptStruct* expectedType, int16& outIndex)
	{
		outIndex = INDEX_NONE;

		const int32 dataID = GetDataID(tag);
		
		if (expectedType == nullptr || !RawStructTypeDatas.IsValidIndex(dataID)) return false;
		if (RawStructTypeDatas[dataID] != expectedType) return false;

		outIndex = dataID;
		return true;
	}
	
	void SwapManualBuffer(int32 indexA, int32 indexB)
	{
		for (int32 i = 0; i < RawDataBuffers.Num(); ++i)
		{
			uint8* buffer = RawDataBuffers[i];
			int32 elementSize = RawElementSizes[i]; 
        
			if (!buffer || elementSize <= 0) continue;

			void* ptrA = buffer + (indexA * elementSize);
			void* ptrB = buffer + (indexB * elementSize);

			FMemory::Memswap(ptrA, ptrB, elementSize);
		}
	}
	
	int32 GetActiveEntityCount() const { return ActiveEntityCount; }
	void SetActiveEntityCount(int32 count) { ActiveEntityCount = count; }
	int32 GetTotalEntityCount() const { return TotalEntityCount; }
	void SetTotalEntityCount(int32 count) { TotalEntityCount = count; }
	int32 GetDataID(const FGameplayTag tag) const
	{
		int32 index = 0;
		for (const auto& pair : DataID)
		{
			if (pair == tag) return index;
			index++;
		}

		return INDEX_NONE;
	}

	bool IsValidBufferIndex(const int32 dataID) const
	{
		return RawDataBuffers.IsValidIndex(dataID);
	}

	/** Allocated element count for one buffer, independent of the configured pool size. */
	int32 GetElementCount(const int32 dataID) const
	{
		return RawElementCounts.IsValidIndex(dataID) ? RawElementCounts[dataID] : 0;
	}

	uint8* GetRawBufferData(const int32 dataID) const
	{
		checkf(DataID.Num() == RawDataBuffers.Num() &&
			DataID.Num() == RawElementSizes.Num() &&
			DataID.Num() == RawStructTypeDatas.Num() &&
			DataID.Num() == RawElementCounts.Num(),
			TEXT("FluxPrime data store arrays are out of sync: DataID=%d buffers=%d sizes=%d types=%d counts=%d"),
			DataID.Num(), RawDataBuffers.Num(), RawElementSizes.Num(), RawStructTypeDatas.Num(), RawElementCounts.Num());

		if (!RawDataBuffers.IsValidIndex(dataID))
		{
			checkf(false, TEXT("FluxPrime data store has no buffer for index %d (buffer count %d)."),
				dataID, RawDataBuffers.Num());
			return nullptr;
		}

		return RawDataBuffers[dataID];
	}
	
	// Rows address the current active prefix, not stable entity handles.
};