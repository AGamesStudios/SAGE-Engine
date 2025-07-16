# Feather Core

The NanoCore is implemented in Rust (edition 2021) under `rust/feather_core`.
It exposes a small C-ABI for integration with Python or other languages.

Build with:

```bash
cargo build --release
```

This produces a shared library that the engine can load at runtime.

The API currently exposes constructors and destructors for `FeatherCore`
and a minimal `ChronoPatchTree` structure used to track reversible changes.
