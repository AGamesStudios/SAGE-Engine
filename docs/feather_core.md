# Feather Core

The NanoCore is implemented in Rust (edition 2021) under `rust/feather_core`.
It exposes a small C-ABI for integration with Python or other languages.

Build with:

```bash
cargo build --release
```

This produces a shared library that the engine can load at runtime.

The API currently exposes constructors and destructors for `FeatherCore`
and a basic `ChronoPatchTree` implementation. The tree stores object states
in an mmap'd file using 1 KB tiles. Each modification records a compressed
delta patch (LZMA) so changes can be undone and redone efficiently.
See the tests for an example of writing data, reverting and reapplying patches.
