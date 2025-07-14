"""Stub NanoCore when the Rust extension is missing."""

def merge_chunk_delta(a: bytes, b: bytes) -> bytes:
    return a + b


def alloc_smart_slice(size: int) -> bytearray:
    return bytearray(1024)


def reset_tree() -> None:
    """No-op stub when the native module is missing."""
    return None
