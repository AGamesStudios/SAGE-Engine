"""Simple window subsystem used for tests."""
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
    surface: Optional[object] = None

    def resize(self, width: int, height: int) -> None:
        self.width = width
        self.height = height
        self.surface = None
        emit("window_resize", {"width": width, "height": height})

    def close(self) -> None:
        global _closed
        self.should_close = True
        _closed = True
        emit("window_close", None)

    def poll(self) -> None:
        pass

    def present(self) -> None:
        pass


_initialized = False
_window: Optional[Window] = None
_closed = False


def boot() -> None:
    """Create the main application window."""
    global _initialized, _window, _closed
    cfg = load_window_config()
    print("[window] create window:", cfg.get("title", "SAGE"))
    _window = Window(**cfg, surface=None)
    _initialized = True
    _closed = False
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


def get_size() -> tuple[int, int]:
    win = get_window()
    return win.width, win.height


def get_title() -> str:
    win = get_window()
    return win.title


def poll() -> None:
    get_window().poll()


def present() -> None:
    get_window().present()


def should_close() -> bool:
    return _closed


def is_initialized() -> bool:
    return _initialized


__all__ = [
    "boot",
    "reset",
    "destroy",
    "get_window",
    "is_initialized",
    "poll",
    "present",
    "should_close",
    "get_size",
    "get_title",
    "Window",
]
