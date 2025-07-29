"""Resource loading, caching and packing utilities."""

import asyncio
from .manager import load, get, preload
from .packer import pack

__all__ = [
    "load",
    "get",
    "preload",
    "pack",
]


async def load_async(path: str) -> bytes:
    await asyncio.sleep(0)
    return load(path)

