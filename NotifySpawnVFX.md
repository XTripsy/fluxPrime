# Notify Spawn VFX asset setup (UE 5.6)

`FFluxPrimeCrowdsNotifySpawnVFX` is independent of projectile abilities, damage and callbacks.

1. Assign `VFXSystem` to a Niagara system authored as a persistent NDC listener. A conventional one-shot burst system is not compatible.
2. Assign a **Global** Niagara Data Channel to `DataChannel`. Add exactly one variable matching `PositionVariable` (default `Position`) with Niagara **Position** type, not Vector. Island channels are rejected because the listener lives at the world origin.
3. Configure the system's NDC reader to read that same channel. C++ validates the channel schema but cannot prove the selected Niagara graph actually consumes it. Use a dedicated channel per visual effect; do not share a projectile channel or add another manually placed listener to it.
4. Keep the system/emitter running indefinitely. Consume incoming records each update and spawn particles once per record, copying the world-space Position into particle position. Do not repeatedly spawn from retained records. No per-event component activation/reset is performed while a listener is running.
5. The component uses `ForceTickLast`. Publish notifies before Niagara's final tick group for same-frame consumption, or configure previous-frame reading consistently for one-frame latency. Verify the chosen CPU/GPU reader's frame semantics in the actual Niagara graph; GPU visibility is enabled as well as CPU visibility.
6. Use world-space particles and bounds covering the whole supported gameplay area, including particle travel. The shared component is at the origin, not at each event. Configure fixed bounds and Niagara scalability/culling so an off-screen origin cannot suppress remote events.

One world subsystem owns one component per channel, shared across all matching notify fragments. A different system or Position variable on an already-owned channel is rejected to prevent double rendering. Use a separate channel for another effect. Components are created lazily on first valid event, never auto-destroy, and are destroyed at subsystem deinitialization. Explicit `ShutdownListeners()` releases all listeners; subsequent events can recreate them. An externally unregistered listener rejects events rather than creating duplicates.

The world pause subsystem registers these components automatically. World pause leaves notify requests pending. A component's explicit custom `SetPaused(true)` is preserved and prevents publication; matching notifies are still consumed, so they do not replay after custom resume. Invalid configuration, writer failure, conflicts and unavailable listeners likewise consume matching cosmetic requests. `TraceHeightOffset` remains a world-up offset in centimeters.

NullRHI tests cover ownership and guards using `/Game/N_DummyDead` as a component lifecycle fixture, not as proof of NDC particle rendering. Verify rendering, bounds, event consumption and CPU/GPU tick ordering with the production NDC-compatible asset in gameplay.
