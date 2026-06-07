Berikut versi README yang menurutku cukup cocok untuk GitHub project yang masih aktif dikembangkan. Gaya bahasanya lebih profesional dan menarik buat recruiter, programmer Unreal, atau developer lain yang mampir ke repository.

---

# Flux Prime

> High-performance crowd simulation framework for Unreal Engine built with ECS, Data-Oriented Design (DOD), VAT, and Instanced Static Mesh Components.

## Overview

**Flux Prime** is an experimental Unreal Engine plugin focused on simulating and rendering large-scale crowds efficiently.

The project is designed around **Entity Component System (ECS)** and **Data-Oriented Design (DOD)** principles, allowing thousands of crowd agents to be updated with minimal overhead while maintaining a modular architecture.

Although originally developed for zombie-style crowd simulations, the framework is flexible enough to support various large-scale agent scenarios such as:

* Zombie hordes
* NPC crowds
* RTS units
* Swarm simulations
* Background population systems

---

## Core Goals

* High-performance crowd simulation
* Fully modular systems
* Cache-friendly data layouts
* Easy integration into Unreal Engine projects
* Massive agent counts with low CPU overhead
* Visual scalability through VAT and instancing

---

## Architecture

Flux Prime follows a modular ECS-inspired approach where each simulation feature is implemented as an independent system.

Examples:

```
Flux Prime
│
├── Movement System
├── Spatial Grid System
├── Animation System
├── State Machine System
├── Collision System
├── Perception System
└── Custom Gameplay Systems
```

Systems can be:

* Added
* Removed
* Replaced
* Extended

without affecting the rest of the simulation.

This allows developers to build only the functionality they need.

---

## Features

### Entity-Based Simulation

Crowd agents are stored as lightweight data rather than traditional Unreal Actors.

Benefits:

* Reduced memory usage
* Better cache locality
* Faster iteration over agents
* Lower CPU cost

---

### Data-Oriented Design

The framework prioritizes:

* Sequential memory access
* Structure-of-Arrays (SoA) layouts
* Batch processing
* SIMD-friendly workflows

This helps maximize performance when updating thousands of entities.

---

### Modular Systems

Every feature is implemented as a standalone module.

Current modules include:

* Movement
* Spatial Grid
* Animation
* State Machine
* Crowd Queries

Additional modules can be developed independently.

---

### Spatial Grid Acceleration

A spatial partitioning system is used to efficiently query nearby agents.

Use cases:

* Neighbor search
* Avoidance
* Target detection
* Local interactions

---

### VAT Animation

Flux Prime uses **Vertex Animation Textures (VAT)** for animation playback.

Advantages:

* No skeletal mesh evaluation per agent
* Significantly reduced CPU animation cost
* Better scalability for large crowds

---

### Instanced Rendering

Rendering is handled using:

* Instanced Static Mesh Components (ISMC)
* VAT Materials

This allows thousands of visible agents to be rendered efficiently while minimizing draw calls.

---

## Design Philosophy

Traditional Unreal Actor-based crowds can become expensive as agent counts increase.

Flux Prime takes a different approach:

```
Gameplay Logic
       ↓
 ECS Simulation
       ↓
 Crowd Data
       ↓
 VAT Animation
       ↓
 ISMC Rendering
```

By separating simulation from visualization, the framework can scale to significantly larger crowds.

---

## Technologies

* Unreal Engine 5
* C++
* ECS-inspired architecture
* Data-Oriented Design
* Vertex Animation Textures (VAT)
* Instanced Static Mesh Components (ISMC)

---

## Vision

Flux Prime aims to become a lightweight and extensible crowd simulation framework that combines the flexibility of ECS with the rendering power of Unreal Engine, making large-scale crowd scenarios accessible without requiring a full custom engine.

---

Karena project ECS/DOD itu biasanya langsung dinilai dari screenshot dan benchmark. Bahkan benchmark sering lebih menarik daripada penjelasan teknis panjang.
