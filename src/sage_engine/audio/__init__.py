from __future__ import annotations

import threading
import time
from dataclasses import dataclass


@dataclass
class AudioHandle:
    path: str
    loop: bool = False
    pan: float = 0.0
    pitch: float = 1.0
    _playing: bool = False
    _thread: threading.Thread | None = None

    def _play_loop(self) -> None:
        self._playing = True
        if not self.loop:
            time.sleep(0.01)
            self._playing = False
            return
        while self._playing:
            time.sleep(0.01)

    def play(self) -> None:
        if self._thread and self._thread.is_alive():
            return
        self._thread = threading.Thread(target=self._play_loop, daemon=True)
        self._thread.start()

    def stop(self) -> None:
        self._playing = False

    def is_playing(self) -> bool:
        return self._playing


def play(path: str, *, loop: bool = False, pan: float = 0.0, pitch: float = 1.0) -> AudioHandle:
    """Play an audio file and return a handle."""
    handle = AudioHandle(path=path, loop=loop, pan=pan, pitch=pitch)
    handle.play()
    return handle

__all__ = ["AudioHandle", "play"]
