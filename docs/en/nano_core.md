# NanoCore

`NanoCore` is a tiny Rust component that exposes high performance helpers to Python.
Set ``SAGE_BUILD=debug`` for a debug build or ``SAGE_BUILD=release`` for an optimised one.
`python tools/build_nano_core.py` uses this variable when calling `maturin` so the wheel is installed
into the active environment. Release wheels are compressed with ``upx`` if available.
After compilation the module can be imported with
`import nano_core`.

Two functions are provided:

* `merge_chunk_delta(a: bytes, b: bytes) -> bytes` – merge two byte sequences.
* `alloc_smart_slice(count: int) -> bytearray` – allocate `count` 1 KiB blocks.
* `reset_tree() -> None` – free all SmartSlice pages for a clean reload.

Both functions release the Python GIL while performing work so they can be
called from multiple threads.
