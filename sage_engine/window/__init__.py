"""Cross-platform window management based on Tkinter with headless fallback."""
from __future__ import annotations

import os
from dataclasses import dataclass

try:
    import tkinter as tk
except Exception:  # pragma: no cover - no display available
    tk = None

from ..events import dispatcher as events

WIN_CLOSE = 1
WIN_RESIZE = 2
WIN_KEY = 3
WIN_MOUSE = 4

_root = None
_should_close = False
_size = (0, 0)


def _on_close() -> None:
    global _should_close
    _should_close = True
    events.emit(WIN_CLOSE)


def _on_configure(event) -> None:  # pragma: no cover - tk only
    global _size
    _size = (event.width, event.height)
    events.emit(WIN_RESIZE, event.width, event.height)


def _on_key(event) -> None:  # pragma: no cover - tk only
    """Keyboard event handler."""
    events.emit(WIN_KEY, event.keysym, event.keycode)


def _on_mouse(event) -> None:  # pragma: no cover - tk only
    """Mouse event handler."""
    events.emit(WIN_MOUSE, event.type, event.x, event.y, getattr(event, "button", 0))


@dataclass
class HeadlessWindow:
    width: int
    height: int

    def update(self) -> None:
        pass

    def destroy(self) -> None:
        pass

    def winfo_width(self) -> int:
        return self.width

    def winfo_height(self) -> int:
        return self.height

    def winfo_id(self) -> int:
        return 0


def init(
    title: str,
    width: int,
    height: int,
    fullscreen: bool = False,
    resizable: bool = True,
    borderless: bool = False,
) -> None:
    """Create the main application window."""
    global _root, _should_close, _size
    _should_close = False
    _size = (width, height)

    headless = os.environ.get("SAGE_HEADLESS") == "1" or tk is None
    if headless:
        _root = HeadlessWindow(width, height)
        return

    try:
        _root = tk.Tk()
    except Exception:  # pragma: no cover - cannot create window
        _root = HeadlessWindow(width, height)
        return

    _root.title(title)
    _root.geometry(f"{width}x{height}")
    if fullscreen:
        _root.attributes("-fullscreen", True)
    if not resizable:
        _root.resizable(False, False)
    if borderless:
        _root.overrideredirect(True)
    _root.protocol("WM_DELETE_WINDOW", _on_close)
    _root.bind("<Configure>", _on_configure)
    _root.bind("<Key>", _on_key)
    _root.bind("<Button>", _on_mouse)
    _root.bind("<Motion>", _on_mouse)


def poll_events() -> None:
    """Process OS events for the window."""
    if _root is None:
        return
    try:
        _root.update_idletasks()
        _root.update()
    except Exception:  # pragma: no cover - ignore closed windows
        pass


def get_size() -> tuple[int, int]:
    return _size


def should_close() -> bool:
    return _should_close


def shutdown() -> None:
    global _root
    if _root is None:
        return
    try:
        _root.destroy()
    except Exception:  # pragma: no cover - already gone
        pass
    _root = None


def get_window_handle() -> int | None:
    if _root is None:
        return None
    try:
        return _root.winfo_id()
    except Exception:  # pragma: no cover - headless
        return None
