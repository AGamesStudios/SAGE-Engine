# SAGE Engine Architecture

This document describes the high level architecture for SAGE Engine based on the SAGE Feather 1.0v specification.

## Modules

- **core** – entry point and dependency registry. Provides `core_boot`, `core_tick`, `core_reset` and `core_shutdown`.
- **time** – time management. Tracks delta time and frame counters.
- **timers** – simple timer manager without allocations in the update loop.
- **events** – event dispatcher with ring buffer.
- **scene** – scene graph storing objects as IDs with roles.
- **roles** – role schemas such as sprite or camera. Schemas are organised into
  *categories* grouping related fields.
- **tasks** – lightweight task scheduler for background logic.
- **resource** – asynchronous resource loading helpers.
- **render** – prepare/sort/flush rendering pipeline.
- **settings** – engine configuration including thread limits.

The engine executes in well defined phases ordered as a DAG. Each module registers its callbacks for update or draw phases.

All modifications to scene objects happen via a `SceneEdit` transaction that is applied atomically between frames.

The design tries to minimize allocations during the update/draw loop and favors a Struct-of-Arrays layout for cache efficiency.
