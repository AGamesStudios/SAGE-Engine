"""Stub NanoCore when the Rust extension is missing."""

def merge_chunk_delta(a: bytes, b: bytes) -> bytes:
    return a + b


def alloc_smart_slice(size: int) -> bytearray:
    return bytearray(size)
