"""Asynchronous resource loading stubs."""

import asyncio
from pathlib import Path


async def load_async(path: str) -> bytes:
    await asyncio.sleep(0)  # simulate async I/O
    with open(Path(path), 'rb') as fh:
        return fh.read()
