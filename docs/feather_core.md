# Feather Core

The NanoCore is implemented in Rust (edition 2021) under `rust/feather_core`.
It exposes a small C-ABI for integration with Python or other languages.

Build with:

```bash
cargo build --release
```

### Python Compatibility
Feather Core depends on PyO3 0.22 which supports Python up to 3.13. Cargo automatically detects your Python interpreter. If you need a newer version, set `PYO3_USE_ABI3_FORWARD_COMPATIBILITY=1` when building.

This produces a shared library that the engine can load at runtime.

The API currently exposes constructors and destructors for `FeatherCore`
and a basic `ChronoPatchTree` implementation. The tree stores object states
in an mmap'd file using 1 KB tiles. Each modification records a compressed
delta patch (LZMA) so changes can be undone and redone efficiently.
See the tests for an example of writing data, reverting and reapplying patches.

## DAG Scheduler

Feather Core also provides a small task scheduler based on a directed acyclic graph. Tasks are registered with a C callback and optional data pointer. Dependencies between tasks are described by task IDs. When executed, the scheduler automatically runs tasks in topological order and detects cycles. Independent tasks run in parallel on separate threads.
