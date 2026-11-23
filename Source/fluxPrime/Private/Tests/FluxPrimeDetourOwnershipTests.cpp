#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Generics/Systems/Navigation/FluxPrimeDetourOwner.h"
#include "Generics/Systems/Navigation/FluxPrimeNavigationSystem.h"
#include "StructUtils/InstancedStruct.h"

namespace
{
struct FDetourTestNavigationSystem : FFluxPrimeNavigationSystem
{
	bool HasDetour() const { return Detour.IsValid(); }
	dtNavMesh* GetBoundNavMesh() const { return Detour.GetBoundNavMesh(); }
	dtNavMeshQuery* GetQuery() const { return Detour.GetQuery(); }
	bool InitializeDetour(dtNavMesh* NavMesh) { return Detour.Initialize(NavMesh, 64, 32); }
};

dtNavMesh* MakeEmptyNavMesh()
{
	dtNavMesh* NavMesh = dtAllocNavMesh();
	if (!NavMesh) return nullptr;

	dtNavMeshParams Params;
	FMemory::Memzero(&Params, sizeof(Params));
	Params.walkableHeight = 144.0f;
	Params.walkableRadius = 34.0f;
	Params.walkableClimb = 40.0f;
	Params.tileWidth = 1.0f;
	Params.tileHeight = 1.0f;
	Params.maxTiles = 1;
	Params.maxPolys = 8;

	for (int32 resolution = 0; resolution < DT_RESOLUTION_COUNT; ++resolution)
	{
		Params.resolutionParams[resolution].bvQuantFactor = 1.0f / 19.0f;
	}

	if (dtStatusFailed(NavMesh->init(&Params)))
	{
		dtFreeNavMesh(NavMesh);
		return nullptr;
	}

	return NavMesh;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFluxPrimeDetourOwnershipTest, "FluxPrime.High.NavigationDetourOwnership",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFluxPrimeDetourOwnershipTest::RunTest(const FString& Parameters)
{
	FDetourTestNavigationSystem system;
	TestFalse(TEXT("A default system owns no Detour state"), system.HasDetour());
	TestFalse(TEXT("A null navmesh is rejected"), system.InitializeDetour(nullptr));
	TestFalse(TEXT("Rejected initialization leaves no state"), system.HasDetour());

	dtNavMesh* navMesh = MakeEmptyNavMesh();
	if (!TestNotNull(TEXT("An empty navmesh is constructible"), navMesh)) return false;

	TestTrue(TEXT("A valid navmesh allocates the query and corridor"), system.InitializeDetour(navMesh));
	TestTrue(TEXT("The owner reports valid state"), system.HasDetour());
	TestTrue(TEXT("The owner remembers the bound navmesh"), system.GetBoundNavMesh() == navMesh);

	FDetourTestNavigationSystem copied = system;
	TestTrue(TEXT("The source keeps its Detour state"), system.HasDetour());
	TestFalse(TEXT("A copy never shares a Detour allocation"), copied.HasDetour());

	FDetourTestNavigationSystem moved = MoveTemp(system);
	TestTrue(TEXT("Move transfers ownership"), moved.HasDetour());
	TestFalse(TEXT("Move leaves the source empty"), system.HasDetour());
	TestTrue(TEXT("Move transfers the navmesh binding"), moved.GetBoundNavMesh() == navMesh);

	FDetourTestNavigationSystem copySource;
	copySource.InitializeDetour(navMesh);
	FDetourTestNavigationSystem copyDestination;
	copyDestination.InitializeDetour(navMesh);
	dtNavMeshQuery* ownQuery = copyDestination.GetQuery();
	copyDestination = *&copyDestination;
	TestTrue(TEXT("Self assignment preserves the query"), copyDestination.GetQuery() == ownQuery);

	copyDestination = copySource;
	TestFalse(TEXT("Copy assignment clears destination runtime ownership"), copyDestination.HasDetour());
	TestNull(TEXT("Copy assignment releases the destination query"), copyDestination.GetQuery());
	TestTrue(TEXT("Copy assignment does not share the source allocation"), copyDestination.GetQuery() != copySource.GetQuery());

	FDetourTestNavigationSystem reassigned;
	reassigned.InitializeDetour(navMesh);
	reassigned = MoveTemp(moved);
	TestTrue(TEXT("Move assignment leaves the destination valid"), reassigned.HasDetour());
	TestFalse(TEXT("Move assignment leaves the source empty"), moved.HasDetour());

	for (int32 cycle = 0; cycle < 5; ++cycle)
	{
		FDetourTestNavigationSystem transient;
		transient.InitializeDetour(navMesh);
	}

	reassigned = FDetourTestNavigationSystem();
	TestFalse(TEXT("Destruction releases the Detour state"), reassigned.HasDetour());

	{
		FInstancedStruct source = FInstancedStruct::Make<FFluxPrimeNavigationSystem>();
		FInstancedStruct destination = FInstancedStruct::Make<FFluxPrimeNavigationSystem>();
		source.GetMutable<FFluxPrimeNavigationSystem>().Detour.Initialize(navMesh, 64, 32);
		destination.GetMutable<FFluxPrimeNavigationSystem>().Detour.Initialize(navMesh, 64, 32);
		destination = source;
		TestFalse(TEXT("Real FInstancedStruct assignment clears destination ownership"), destination.Get<FFluxPrimeNavigationSystem>().Detour.IsValid());
		TestTrue(TEXT("Real FInstancedStruct assignment preserves source ownership"), source.Get<FFluxPrimeNavigationSystem>().Detour.IsValid());
		FInstancedStruct copiedStruct(source);
		TestFalse(TEXT("Real FInstancedStruct construction copies no owner"), copiedStruct.Get<FFluxPrimeNavigationSystem>().Detour.IsValid());
	}

	FFluxPrimeNavigationSystem noRecast;
	noRecast.Detour.Initialize(navMesh, 64, 32);
	TestFalse(TEXT("Allocated query without Recast is not executable"), noRecast.EnsureDetourMatchesNavMesh());
	TestFalse(TEXT("Missing Recast releases ownership"), noRecast.Detour.IsValid());
	noRecast.RecastNavMesh = GetMutableDefault<ARecastNavMesh>();
	noRecast.Detour.Initialize(navMesh, 64, 32);
	TestFalse(TEXT("Default Recast is not executable"), noRecast.EnsureDetourMatchesNavMesh());

	FFluxPrimeDetourOwner owner;
	owner.Initialize(navMesh, 64, 32);
	TestTrue(TEXT("Unchanged mesh matches the snapshot"), owner.MatchesUnchangedMesh(navMesh));
	dtNavMesh* replacement = MakeEmptyNavMesh();
	TestFalse(TEXT("Replacement mesh is rejected rather than rebound"), owner.MatchesUnchangedMesh(replacement));
	dtFreeNavMesh(replacement);

	// Simulate Detour's tile salt change on removal/replacement without changing the mesh address.
	dtMeshTile* tile = const_cast<dtMeshTile*>(static_cast<const dtNavMesh*>(navMesh)->getTile(0));
	dtMeshHeader header{};
	tile->header = &header;
	owner.Initialize(navMesh, 64, 32);
	TestTrue(TEXT("Populated tile snapshot matches"), owner.MatchesUnchangedMesh(navMesh));
	const auto oldSalt = tile->salt;
	++tile->salt;
	TestFalse(TEXT("In-place tile replacement is rejected"), owner.MatchesUnchangedMesh(navMesh));
	tile->salt = oldSalt;
	tile->header = nullptr;
	TestFalse(TEXT("In-place tile removal is rejected"), owner.MatchesUnchangedMesh(navMesh));
	owner.Reset();
	copySource = FDetourTestNavigationSystem();
	dtFreeNavMesh(navMesh);
	return true;
}
#endif
