from __future__ import annotations

import time
from dataclasses import dataclass

from ...events import dispatcher as events
from .. import WIN_CLOSE, WIN_RESIZE, WIN_KEY, WIN_MOUSE


@dataclass
class CocoaWindow:
    title: str
    width: int
    height: int
    fullscreen: bool
    resizable: bool
    borderless: bool
    _closed: bool = False

    def __post_init__(self) -> None:
        # Placeholder for macOS Cocoa window creation
        pass

    def poll(self) -> None:
        # Placeholder event polling for Cocoa
        time.sleep(0.001)

    def destroy(self) -> None:
        pass

    def get_handle(self):
        return 0

    def get_framebuffer(self) -> bytearray | None:
        return None

    def get_size(self) -> tuple[int, int]:
        return (self.width, self.height)

    def should_close(self) -> bool:
        return self._closed

    def _on_close(self):
        self._closed = True
        events.emit(WIN_CLOSE)

    def _on_resize(self, width: int, height: int):
        self.width = width
        self.height = height
        events.emit(WIN_RESIZE, width, height)

    def _on_key(self, key: int):
        events.emit(WIN_KEY, key, key)

    def _on_mouse(self, typ: str, x: int, y: int, button: int):
        events.emit(WIN_MOUSE, typ, x, y, button)

    def set_fullscreen(self, enabled: bool) -> None:
        self.fullscreen = enabled

    def set_resolution(self, width: int, height: int) -> None:
        self.width = width
        self.height = height

