# fluxPrime

> High-performance, ECS-inspired crowd simulation framework for Unreal Engine 5 built with Data-Oriented Design (DOD), tag-addressed Structure-of-Arrays (SoA), Vertex Animation Textures (VAT), and Instanced Static Mesh Components (ISMC).

**Current Version:** `0.2.1` (Experimental)  
**Target Engine:** Unreal Engine 5.6  

---

## Overview

**fluxPrime** is an experimental Unreal Engine plugin designed to simulate, network, and render large-scale crowds efficiently without the lifecycle, ticking, or memory overhead of per-agent `AActor` or `UObject` instances.

Instead of treating each crowd agent as an individual Actor, **fluxPrime** represents agents as lightweight rows inside contiguous, archetype-pooled **Structure-of-Arrays (SoA)** memory buffers. Gameplay designers compose data schemas, state machines, and simulation pipelines using `UPrimaryDataAsset` and `FInstancedStruct` presets in the Editor, while the C++ runtime executes cache-friendly, struct-based systems over active entity ranges.

Suitable for large-scale scenarios such as:
- Zombie hordes & survival waves
- Ambient NPC crowds
- RTS & large-scale tactical units
- Swarm & boid simulations

---

## Core Goals

- **Cache-Friendly Runtime:** Contiguous SoA memory buffers per archetype with swap-based data compaction (`O(1)` removal without heap fragmentation).
- **Designer-Friendly Authoring:** Modular `FInstancedStruct` system/state fragments stored in `UPrimaryDataAsset` presets—add, remove, or swap systems per configuration without touching C++.
- **Server-Authoritative Multiplayer:** Asymmetric ("Dumb Client") initialization with quantized, spatial-filtered, batched snapshot replication.
- **Physics-Free Crowd Queries:** Grid-accelerated mathematical raycasting, Boids avoidance, and 2.5D baked terrain height lookups instead of heavy per-agent Chaos Physics bodies.
- **Scalable Rendering:** Zero skeletal mesh evaluation via Unreal's `AnimToTexture` (VAT) paired with batched `InstancedStaticMeshComponent` (ISMC) transform updates.

---

## Architecture

**fluxPrime** separates authoring assets (`UPrimaryDataAsset`), world orchestration (`AActor` / `UActorComponent` / `UWorldSubsystem`), and pure C++ `USTRUCT` simulation systems:

```text
Authoring Layer (UPrimaryDataAsset + FInstancedStruct)
 ├── UFluxPrimeEntityCatalogIdentity   (Archetype mesh, stats, pool capacity)
 ├── UFluxPrimeDataStoreIdentity       (SoA GameplayTag -> Component type schema)
 ├── UFluxPrimeStateIdentity           (FSM states & transition fragments)
 ├── UManagerConfiguration             (Runtime module fragments)
 └── USystemsConfiguration             (Runtime system fragments & mesh orientation)
        │
        ▼
Orchestration & Networking Layer
 ├── AFluxPrimeCrowdsController        (Level host actor & authority delegate wiring)
 ├── UFluxPrimeCrowdsManager           (Owns archetype DataStores, module/system ticks)
 ├── UFluxPrimeNetManager              (Server-to-client Unreliable Multicast snapshots)
 ├── UFluxPrimeCrowdsTargetManager     (Player-owned component for Server Reliable RPCs)
 └── UFluxPrimeCrowdsWorldSubsystem    (External gameplay facade via FInstancedStruct payloads)
        │
        ▼
Simulation & Rendering Pipeline (USTRUCT Systems over SoA Buffers)
 ├── Core / Generic Systems
 │    ├── FFluxPrimeSpatialGridSystem         (Prefix-sum dense spatial partitioning)
 │    ├── FFluxPrimeNavigationSystem          (Recast/Detour pathfinding & corridors)
 │    ├── FFluxPrimeBoidsSystem               (Spatial-grid neighbor separation)
 │    ├── FFluxPrimeMovementSystem            (Kinematic steering & velocity integration)
 │    ├── FFluxPrimeGroundHeightSystem        (2.5D multi-layer baked terrain height)
 │    ├── FFluxPrimeRaycastSystem             (Grid-accelerated segment-to-point hit query)
 │    ├── FFluxPrimeHealthSystem              (Contiguous damage accumulation & health)
 │    ├── FFluxPrimeAnimationSystem           (VAT frame playback & notify dispatching)
 │    ├── FFluxPrimeVisualInterpolationSystem (Client-side VInterpTo / FInterpTo smoothing)
 │    └── FFluxPrimeRenderSystem              (Batched ISMC transform updates)
 └── Crowds Domain Systems
      ├── FFluxPrimeCrowdsStateMachineSystem  (Integer-indexed FSM & transition evaluation)
      ├── FFluxPrimeCrowdsTargetSystem        (Priority, load balancing & target stickiness)
      ├── FFluxPrimeCrowdsAbilitySystem       (Melee sweep & Niagara projectile abilities)
      ├── FFluxPrimeCrowdsSnapshotSystem      (10 Hz quantized batched network snapshots)
      ├── FFluxPrimeCrowdsKillEventSystem     (Player kill tracking via FUniqueNetIdRepl)
      └── FFluxPrimeCrowdsCompactSystem       (Swap-based pool recycling)
```

---

## Key Features

### Tag-Addressed SoA & Object Pooling
- Each archetype allocates a fixed-capacity `FFluxPrimeArchetypeDataStore` with manual raw buffers mapped by `FGameplayTag` during initialization (`O(1)` column pointer access inside hot loops).
- Dead or despawned agents are recycled via `SwapManualBuffer`, swapping the tail active entity into the vacated slot to keep active entities packed in `[0, ActiveEntityCount)`.

### Server-Authoritative ("Dumb Client") Networking
- **Asymmetric Initialization:** Server and client instances allocate only the SoA buffers and register only the systems required for their respective network authority.
- **Batched Snapshots:** `FFluxPrimeCrowdsSnapshotSystem` replicates entity states at a configurable rate (default `10 Hz`) using `FVector_NetQuantize10` compression, spatial grid relevancy filtering, and packet batching (`128` entities per batch).
- **Client Visual Interpolation:** Clients smoothly interpolate position and yaw toward replicated waypoints without running local AI, Boids, or Detour navigation.

### Spatial Grid & Physics-Free Combat
- **Prefix-Sum Spatial Grid:** Rebuilds dense spatial cells every frame (`GridCounts`, `GridOffsets`, `GridMembers`) to accelerate Boids separation, network relevancy, and hit detection.
- **Mathematical Raycast:** `FFluxPrimeRaycastSystem` evaluates segment-to-point intersections directly against spatial grid cells, eliminating the need for Chaos Physics collision on thousands of ISMC instances.
- **Abilities & Niagara Integration:** Supports Melee sphere-sweep abilities and Niagara particle projectiles (including an opt-in batched Niagara Data Channel pipeline with per-particle damage export).

### 2.5D Multi-Layer Baked Ground Height
- `FFluxPrimeGroundHeightSystem` pre-bakes multi-floor terrain heights into a contiguous 2.5D grid on initialization, allowing `O(1)` runtime vertical ground alignment across uneven terrain and multi-story structures.

### VAT Animation & Animation Notifies
- Integrates with Unreal Engine's **AnimToTexture** plugin to drive skeletal animations via Vertex Animation Textures on ISMC instances.
- Supports global state mapping (up to 128 states) and frame-accurate `AnimationNotify` fragments (`Ability`, `Dead`, `SpawnVFX`).

---

## Data & Execution Flow

```text
External Gameplay (Weapons, Player Pawns, Spawners)
       │  (FInstancedStruct Payloads / Server RPCs)
       ▼
UFluxPrimeCrowdsWorldSubsystem ──► UFluxPrimeCrowdsManager
                                          │
                 ┌────────────────────────┴────────────────────────┐
                 ▼                                                 ▼
          [Server Authority]                                [Dumb Client]
  AI Target ─► Detour Nav ─► Boids & Movement          Receive Snapshot Batches
       │                                                           │
  2.5D Ground ─► FSM ─► Ability & Health               Visual Interpolation (Lerp)
       │                                                           │
  Compaction ─► Batched Snapshot (10 Hz)               VAT Animation ─► ISMC Render
```

---

## Technical Stack & Module Dependencies

- **Engine:** Unreal Engine 5.6 (C++)
- **Core Modules:** `Core`, `CoreUObject`, `Engine`, `GameplayTags`, `StructUtils` (`FInstancedStruct`)
- **Navigation:** `NavigationSystem`, `Navmesh` (Recast / Detour)
- **Rendering & VFX:** `AnimToTexture` (VAT), `InstancedStaticMeshComponent` (ISMC), `Niagara`
- **Networking:** `CoreOnline` (`FUniqueNetIdRepl`), Unreal Replication / RPCs

---

## Current Status & Roadmap

**fluxPrime** (`v0.2.1`) is currently an **Experimental** runtime plugin. Core simulation loops execute serially and deterministically on the Game Thread.

### Planned Improvements
- **Multithreading:** Parallel execution (`ParallelFor` / Unreal Task Graph) across independent archetype simulation loops.
- **Rendering Culling & LOD:** CPU-side frustum/distance culling and dynamic ISMC mesh LOD management.
- **FSM Batch Execution:** Grouping entities by state ID to minimize per-entity virtual/function-pointer branching overhead.
- **Navigation Enhancements:** Multi-corner waypoint buffering and shared spatial cell corridors for dense crowds.
- **Audio Notifies & Editor Tooling:** Full audio playback support for `SpawnSFX` notifies and automated asset validation utilities.

---

## Vision

**fluxPrime** aims to provide a transparent, lightweight, and extensible C++ crowd framework for Unreal Engine—bridging the performance of Data-Oriented Design with the accessibility of Unreal's DataAsset and `FInstancedStruct` workflows.
