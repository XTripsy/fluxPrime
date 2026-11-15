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
	TArray<FGameplayTag> DataID;
	
public:
	~FFluxPrimeArchetypeDataStore()
	{
		FreeManualBuffer();
	}
	
	void FreeManualBuffer()
	{
		for (auto& pair : RawDataBuffers)
		{
			FMemory::Free(pair);
		}
		
		RawStructTypeDatas.Empty();
		RawDataBuffers.Empty();
		RawElementSizes.Empty();
		DataID.Empty();
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
	void AddNewDataID(const FGameplayTag tag)
	{
		if (GetDataID(tag) != INDEX_NONE) return;
		
		DataID.Add(tag);
	};
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
	void AddNewRawStructTypeData(UScriptStruct* structType) { RawStructTypeDatas.Add(structType); }
	void AddNewRawBufferData(uint8* allocatedMemory) { RawDataBuffers.Add(allocatedMemory); }
	uint8* GetRawBufferData(const int32 dataID) const
	{
		checkf(DataID.Num() == RawDataBuffers.Num() &&
			DataID.Num() == RawElementSizes.Num() &&
			DataID.Num() == RawStructTypeDatas.Num(),
			TEXT("FluxPrime data store arrays are out of sync: DataID=%d buffers=%d sizes=%d types=%d"),
			DataID.Num(), RawDataBuffers.Num(), RawElementSizes.Num(), RawStructTypeDatas.Num());

		return RawDataBuffers[dataID];
	}
	void AddNewRawElementSizes(int32 elementSizes) { RawElementSizes.Add(elementSizes); }
	
	// Rows address the current active prefix, not stable entity handles.
};