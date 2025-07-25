"""Simple drawing helpers used for debug visuals."""
from __future__ import annotations

from typing import Tuple

_calls: list[tuple] = []
_initialized = False


def boot() -> None:
    """Initialise the draw subsystem."""
    global _initialized
    _initialized = True
    _calls.clear()


def reset() -> None:
    global _initialized
    _initialized = False
    _calls.clear()


def destroy() -> None:
    reset()


def get_calls() -> list:
    return list(_calls)



def _surface() -> None:
    return None


def draw_line(start: tuple[int, int], end: tuple[int, int], color: Tuple[int, int, int] = (255, 255, 255), width: int = 1) -> None:
    _calls.append(("line", start, end, color, width))
    _surface()


def draw_rect(rect: tuple[int, int, int, int], color: Tuple[int, int, int] = (255, 255, 255), width: int = 1) -> None:
    _calls.append(("rect", rect, color, width))
    _surface()


def draw_circle(pos: tuple[int, int], radius: int, color: Tuple[int, int, int] = (255, 255, 255), width: int = 1) -> None:
    _calls.append(("circle", pos, radius, color, width))
    _surface()


__all__ = [
    "boot",
    "reset",
    "destroy",
    "draw_line",
    "draw_rect",
    "draw_circle",
    "get_calls",
]
