# NanoCore

`NanoCore` is a tiny Rust component that exposes high performance helpers to Python.
Build it with `maturin develop --release` so the wheel gets installed into your
current environment. After compilation the module can be imported with
`import nano_core`.

Two functions are provided:

* `merge_chunk_delta(a: bytes, b: bytes) -> bytes` – merge two byte sequences.
* `alloc_smart_slice(count: int) -> bytearray` – allocate `count` 1 KiB blocks.

Both functions release the Python GIL while performing work so they can be
called from multiple threads.
