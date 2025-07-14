from __future__ import annotations

import asyncio
import importlib
from engine import adaptors
import websockets

__all__ = ["start_listener"]


async def _handle(ws):
    async for msg in ws:
        if msg.startswith("reload "):
            mod_name = msg.split(" ", 1)[1]
            try:
                mod = importlib.import_module(mod_name)
                importlib.reload(mod)
                adaptors.load_adaptors([mod_name.split('.')[-1]])
            except Exception:  # pragma: no cover - log and continue
                pass


async def start_listener(host: str = "localhost", port: int = 8765) -> None:
    async with websockets.serve(_handle, host, port):
        await asyncio.Future()
