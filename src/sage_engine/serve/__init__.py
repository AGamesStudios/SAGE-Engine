from __future__ import annotations

import json
from pathlib import Path

from ..build import convert_audio


class DummyWS:
    def __init__(self) -> None:
        self.messages: list[str] = []

    def send(self, msg: str) -> None:
        self.messages.append(msg)


class LiveServer:
    """Very small watcher that rebuilds assets and notifies clients."""

    def __init__(self, watch_dir: str) -> None:
        self.watch_dir = Path(watch_dir)
        self.websockets: list[DummyWS] = []

    def add_client(self, ws: DummyWS) -> None:
        self.websockets.append(ws)

    def handle_change(self, path: str | Path) -> None:
        p = Path(path)
        payload: dict[str, str]
        if p.suffix.lower() in {".ogg", ".wav", ".mp3"}:
            convert_audio(str(p), str(self.watch_dir / "audio_cache"))
            payload = {"path": str(p)}
        elif p.suffix.lower() == ".vel":
            payload = {"type": "theme", "name": p.stem}
        else:
            payload = {"path": str(p)}
        msg = json.dumps({"type": "reload_asset", **payload})
        for ws in list(self.websockets):
            ws.send(msg)


__all__ = ["LiveServer", "DummyWS"]
