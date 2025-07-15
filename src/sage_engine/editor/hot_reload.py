from __future__ import annotations

import asyncio
import importlib
from sage_engine import adaptors
import websockets
from typing import Set

__all__ = ["start_listener"]


_CLIENTS: Set[websockets.WebSocketServerProtocol] = set()


async def _handle(ws):
    _CLIENTS.add(ws)
    try:
        async for msg in ws:
            if msg.startswith("reload "):
                mod_name = msg.split(" ", 1)[1]
                try:
                    mod = importlib.import_module(mod_name)
                    importlib.reload(mod)
                    adaptors.load_adaptors([mod_name.split('.')[-1]])
                except Exception:  # pragma: no cover - log and continue
                    pass
                for client in list(_CLIENTS):
                    if client is not ws:
                        await client.send(msg)
            elif msg.startswith("toast "):
                for client in list(_CLIENTS):
                    if client is not ws:
                        await client.send(msg)
    finally:
        _CLIENTS.discard(ws)


async def start_listener(host: str = "localhost", port: int = 8765) -> None:
    async with websockets.serve(_handle, host, port):
        await asyncio.Future()
