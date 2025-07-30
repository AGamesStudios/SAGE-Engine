"""Resource loading, caching and packing utilities."""

import asyncio
from .manager import load, get, preload
from .packer import pack
from .loader import load_cfg
from .sage_pack import main as pack_cli
from .. import core

__all__ = [
    "load",
    "get",
    "preload",
    "pack",
    "load_async",
    "pack_cli",
    "load_cfg",
]


async def load_async(path: str) -> bytes:
    await asyncio.sleep(0)
    return load(path)

core.expose(
    "resource",
    {
        "load": load,
        "get": get,
        "preload": preload,
        "pack": pack,
        "load_async": load_async,
        "pack_cli": pack_cli,
        "load_cfg": load_cfg,
    },
)

