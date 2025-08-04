"""Win32 stub implementation."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass
class Window:
    title: str
    width: int
    height: int
    closed: bool = False


def create_window(title: str, size: tuple[int, int]) -> Window:
    return Window(title, size[0], size[1])


def process_events(win: Window) -> list[str]:
    return ["window_closed"] if win.closed else []
