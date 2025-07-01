from __future__ import annotations
import time
from typing import Optional

from .scene import Scene

class Engine:
    """Very small game loop managing a single scene."""
    def __init__(self, scene: Optional[Scene] = None, fps: int = 60):
        self.scene = scene or Scene()
        self.fps = fps
        self._frame_interval = 1.0 / fps if fps else 0
        self._running = False

    def run(self) -> None:
        self._running = True
        last = time.perf_counter()
        while self._running:
            now = time.perf_counter()
            dt = now - last
            last = now
            if self.scene:
                self.scene.update(dt)
                self.scene.draw(None)
            if self._frame_interval:
                sleep = self._frame_interval - dt
                if sleep > 0:
                    time.sleep(sleep)

    def stop(self) -> None:
        self._running = False
