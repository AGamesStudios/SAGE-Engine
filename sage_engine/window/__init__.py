"""Minimal window subsystem emitting events."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

from sage.events import emit
from sage.config import load_window_config


@dataclass
class Window:
    width: int
    height: int
    title: str
    vsync: bool
    resizable: bool
    fullscreen: bool
    should_close: bool = False

    def resize(self, width: int, height: int) -> None:
        self.width = width
        self.height = height
        emit("window_resize", {"width": width, "height": height})

    def close(self) -> None:
        self.should_close = True
        emit("window_close", None)


_initialized = False
_window: Optional[Window] = None


def boot() -> None:
    """Create the main application window."""
    global _initialized, _window
    cfg = load_window_config()
    _window = Window(**cfg)
    _initialized = True
    emit("window_create", {"width": _window.width, "height": _window.height})


def reset() -> None:
    global _initialized, _window
    if _window is not None:
        _window.close()
    _initialized = False
    _window = None


def destroy() -> None:
    reset()


def get_window() -> Window:
    assert _window is not None, "window not initialised"
    return _window


def is_initialized() -> bool:
    return _initialized


__all__ = [
    "boot",
    "reset",
    "destroy",
    "get_window",
    "is_initialized",
    "Window",
]
