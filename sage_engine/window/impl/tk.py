from __future__ import annotations

import time

try:
    import tkinter as tk
except Exception:  # pragma: no cover - environments without a display
    tk = None

from ...events import dispatcher as events
from .. import WIN_CLOSE, WIN_RESIZE, WIN_KEY, WIN_MOUSE


class TkWindow:
    """Simple Tkinter-based window backend."""

    def __init__(self, title: str, width: int, height: int, fullscreen: bool, resizable: bool, borderless: bool) -> None:
        self.root = tk.Tk()
        self.root.title(title)
        self.root.geometry(f"{width}x{height}")
        if fullscreen:
            self.root.attributes("-fullscreen", True)
        if not resizable:
            self.root.resizable(False, False)
        if borderless:
            self.root.overrideredirect(True)
        self.width = width
        self.height = height
        self._should_close = False
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)
        self.root.bind("<Configure>", self._on_configure)
        self.root.bind("<Key>", self._on_key)
        self.root.bind("<Button>", self._on_mouse)
        self.root.bind("<Motion>", self._on_mouse)

    def _on_close(self, event=None) -> None:
        self._should_close = True
        events.emit(WIN_CLOSE)

    def _on_configure(self, event) -> None:  # pragma: no cover - tk only
        self.width = event.width
        self.height = event.height
        events.emit(WIN_RESIZE, event.width, event.height)

    def _on_key(self, event) -> None:  # pragma: no cover - tk only
        events.emit(WIN_KEY, event.keysym, event.keycode)

    def _on_mouse(self, event) -> None:  # pragma: no cover - tk only
        events.emit(WIN_MOUSE, event.type, event.x, event.y, getattr(event, "button", 0))

    def poll(self) -> None:
        try:
            self.root.update_idletasks()
            self.root.update()
        except Exception:
            pass
        time.sleep(0.001)  # prevent high CPU usage

    def destroy(self) -> None:
        try:
            self.root.destroy()
        except Exception:
            pass

    def get_handle(self) -> int | None:
        try:
            return self.root.winfo_id()
        except Exception:
            return None
