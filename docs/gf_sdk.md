# SAGE GameFrame SDK

This directory contains a prototype implementation of the GameFrame SDK as a C library. The SDK provides frame metrics collection and simple formula execution. The public headers reside in `gf_sdk/include/sage/gf/` and a minimal core implementation lives under `gf_sdk/src/`.

The library can be built on Linux using `make`:

```bash
cd gf_sdk
make
```

This produces `libgf_sdk.a` which exposes the minimal API in `gf_core.h`.

The code is intentionally small and does not yet implement the full specification. It serves as a starting point for future development.
