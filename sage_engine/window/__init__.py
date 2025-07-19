"""Window subsystem using pygame for display and events."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

import pygame

from sage.events import emit
from sage.config import load_window_config
from sage_engine.input import handle_pygame_event


@dataclass
class Window:
    width: int
    height: int
    title: str
    vsync: bool
    resizable: bool
    fullscreen: bool
    should_close: bool = False
    surface: Optional[pygame.Surface] = None

    def resize(self, width: int, height: int) -> None:
        self.width = width
        self.height = height
        flags = pygame.RESIZABLE if self.resizable else 0
        if self.fullscreen:
            flags |= pygame.FULLSCREEN
        self.surface = pygame.display.set_mode((width, height), flags, vsync=1 if self.vsync else 0)
        emit("window_resize", {"width": width, "height": height})

    def close(self) -> None:
        self.should_close = True
        emit("window_close", None)

    def poll(self) -> None:
        for ev in pygame.event.get():
            if ev.type == pygame.QUIT:
                self.close()
            elif ev.type == pygame.VIDEORESIZE:
                self.resize(ev.w, ev.h)
            else:
                handle_pygame_event(ev)

    def present(self) -> None:
        pygame.display.flip()


_initialized = False
_window: Optional[Window] = None


def boot() -> None:
    """Create the main application window."""
    global _initialized, _window
    cfg = load_window_config()
    pygame.init()
    flags = pygame.RESIZABLE if cfg.get("resizable", False) else 0
    if cfg.get("fullscreen"):
        flags |= pygame.FULLSCREEN
    surface = pygame.display.set_mode(
        (cfg.get("width", 800), cfg.get("height", 600)),
        flags,
        vsync=1 if cfg.get("vsync", True) else 0,
    )
    pygame.display.set_caption(cfg.get("title", "SAGE"))
    print("[window] created window:", cfg.get("title", "SAGE"), (cfg.get("width", 800), cfg.get("height", 600)))
    _window = Window(**cfg, surface=surface)
    _initialized = True
    emit("window_create", {"width": _window.width, "height": _window.height})


def reset() -> None:
    global _initialized, _window
    if _window is not None:
        _window.close()
    pygame.quit()
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
    "get_size",
    "get_title",
    "Window",
]
