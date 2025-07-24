"""Simple drawing helpers used for debug visuals."""
from __future__ import annotations

from typing import Tuple

import pygame

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


def _surface() -> pygame.Surface | None:
    return pygame.display.get_surface() if pygame.get_init() else None


def draw_line(start: tuple[int, int], end: tuple[int, int], color: Tuple[int, int, int] = (255, 255, 255), width: int = 1) -> None:
    _calls.append(("line", start, end, color, width))
    surf = _surface()
    if surf is not None:
        pygame.draw.line(surf, color, start, end, width)


def draw_rect(rect: tuple[int, int, int, int], color: Tuple[int, int, int] = (255, 255, 255), width: int = 1) -> None:
    _calls.append(("rect", rect, color, width))
    surf = _surface()
    if surf is not None:
        pygame.draw.rect(surf, color, rect, width)


def draw_circle(pos: tuple[int, int], radius: int, color: Tuple[int, int, int] = (255, 255, 255), width: int = 1) -> None:
    _calls.append(("circle", pos, radius, color, width))
    surf = _surface()
    if surf is not None:
        pygame.draw.circle(surf, color, pos, radius, width)


__all__ = [
    "boot",
    "reset",
    "destroy",
    "draw_line",
    "draw_rect",
    "draw_circle",
    "get_calls",
]
