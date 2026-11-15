#pragma once

#include "CoreMinimal.h"
#include "Detour/DetourNavMesh.h"
#include "Detour/DetourNavMeshQuery.h"
#include "DetourCrowd/DetourPathCorridor.h"

/**
 * Owns the Detour resources used by a navigation system.
 *
 * Detour types have manual lifetimes, so this wrapper is the single owner:
 * - copies are intentionally empty, because a copy must never share an allocation;
 * - moves transfer ownership and leave the source empty;
 * - destruction frees the query and the corridor.
 *
 * Copyability is required by UE's reflected struct/FInstancedStruct copying.
 * Copies must be initialized separately; runtime allocations are never shared.
 * Tile references are captured to reject tile replacement, including in-place
 * rebuilds where the dtNavMesh pointer does not change.
 */
struct FFluxPrimeDetourOwner
{
public:
	FFluxPrimeDetourOwner() = default;

	FFluxPrimeDetourOwner(const FFluxPrimeDetourOwner&) {}
	FFluxPrimeDetourOwner& operator=(const FFluxPrimeDetourOwner& Other)
	{
		if (this != &Other) Reset();
		return *this;
	}

	FFluxPrimeDetourOwner(FFluxPrimeDetourOwner&& Other) noexcept
	{
		Transfer(Other);
	}

	FFluxPrimeDetourOwner& operator=(FFluxPrimeDetourOwner&& Other) noexcept
	{
		if (this != &Other)
		{
			Reset();
			Transfer(Other);
		}

		return *this;
	}

	~FFluxPrimeDetourOwner()
	{
		Reset();
	}

	bool Initialize(dtNavMesh* NavMesh, const int32 MaxNodes, const int32 MaxPath)
	{
		Reset();
		if (!NavMesh) return false;

		Query = dtAllocNavMeshQuery();
		if (!Query) return false;

		if (dtStatusFailed(Query->init(NavMesh, MaxNodes)))
		{
			Reset();
			return false;
		}

		Corridor = new dtPathCorridor();
		if (!Corridor->init(MaxPath))
		{
			Reset();
			return false;
		}

		BoundNavMesh = NavMesh;
		const dtNavMesh* Mesh = NavMesh;
		TileRefs.Reserve(Mesh->getMaxTiles());
		for (int32 Index = 0; Index < Mesh->getMaxTiles(); ++Index)
		{
			const dtMeshTile* Tile = Mesh->getTile(Index);
			TileRefs.Add(Tile && Tile->header ? Mesh->getTileRef(Tile) : 0);
		}
		return true;
	}

	void Reset()
	{
		if (Query)
		{
			dtFreeNavMeshQuery(Query);
			Query = nullptr;
		}

		delete Corridor;
		Corridor = nullptr;
		BoundNavMesh = nullptr;
		TileRefs.Reset();
	}

	// Call on the game thread before using any stored polygon references.
	bool MatchesUnchangedMesh(const dtNavMesh* Mesh) const
	{
		if (!IsValid() || !Mesh || Mesh != BoundNavMesh || Mesh->getMaxTiles() != TileRefs.Num()) return false;
		for (int32 Index = 0; Index < TileRefs.Num(); ++Index)
		{
			const dtMeshTile* Tile = Mesh->getTile(Index);
			if (TileRefs[Index] != (Tile && Tile->header ? Mesh->getTileRef(Tile) : 0)) return false;
		}
		return true;
	}

	bool IsValid() const { return Query != nullptr && Corridor != nullptr; }
	dtNavMeshQuery* GetQuery() const { return Query; }
	dtNavMesh* GetBoundNavMesh() const { return BoundNavMesh; }

	dtPathCorridor& GetCorridor() const
	{
		check(Corridor);
		return *Corridor;
	}

private:
	void Transfer(FFluxPrimeDetourOwner& Other)
	{
		Query = Other.Query;
		Corridor = Other.Corridor;
		BoundNavMesh = Other.BoundNavMesh;
		TileRefs = MoveTemp(Other.TileRefs);

		Other.Query = nullptr;
		Other.Corridor = nullptr;
		Other.BoundNavMesh = nullptr;
	}

	dtNavMeshQuery* Query = nullptr;
	dtPathCorridor* Corridor = nullptr;
	dtNavMesh* BoundNavMesh = nullptr;
	TArray<dtTileRef> TileRefs;
};
