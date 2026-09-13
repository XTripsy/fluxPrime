#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

struct FFluxPrimeArchetypeDataStore
{
private:
	int32 ActiveEntityCount = 0;
	int32 TotalEntityCount = 0;
	TArray<uint8*> RawDataBuffers;
	TArray<int32> RawElementSizes;
	TArray<int32> EntityLookup;
	TArray<FGameplayTag> DataID;
	
public:
	void SwapManualBuffer(int32 IndexA, int32 IndexB)
	{
		for (int32 i = 0; i < RawDataBuffers.Num(); ++i)
		{
			uint8* Buffer = RawDataBuffers[i];
			int32 ElementSize = RawElementSizes[i]; 
        
			if (!Buffer || ElementSize <= 0) continue;

			void* PtrA = Buffer + (IndexA * ElementSize);
			void* PtrB = Buffer + (IndexB * ElementSize);

			FMemory::Memswap(PtrA, PtrB, ElementSize);
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
	void AddNewRawBufferData(uint8* allocatedMemory) { RawDataBuffers.Add(allocatedMemory); }
	uint8* GetRawBufferData(const int32 dataID) const
	{
		return RawDataBuffers[dataID];
	}
	void AddNewRawElementSizes(int32 elementSizes) { RawElementSizes.Add(elementSizes); }
	
	void InitializedLookUp(int32 countLookUpData) { EntityLookup.Init(0, countLookUpData); }
	void UpdateLookUpValue(int32 indexLookUp, int32 valueLookUp) { EntityLookup[indexLookUp] = valueLookUp; }
	int32 GetIndexDataLookUp(int32 entityId) const { return EntityLookup[entityId]; };
};