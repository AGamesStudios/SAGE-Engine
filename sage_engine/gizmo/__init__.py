"""Basic gizmo utilities for editing and debug."""
from __future__ import annotations

from typing import Tuple

from ..draw import draw_line
from sage_object import SAGEObject

_enabled = True


def boot() -> None:
    pass


def reset() -> None:
    pass


def destroy() -> None:
    pass


def set_enabled(value: bool) -> None:
    global _enabled
    _enabled = value


def is_enabled() -> bool:
    return _enabled


def draw_transform_gizmo(obj: SAGEObject, color: Tuple[int, int, int] = (0, 255, 0)) -> None:
    if not _enabled:
        return
    x = int(obj.params.get("x", 0))
    y = int(obj.params.get("y", 0))
    draw_line((x - 10, y), (x + 10, y), color)
    draw_line((x, y - 10), (x, y + 10), color)


__all__ = [
    "boot",
    "reset",
    "destroy",
    "set_enabled",
    "is_enabled",
    "draw_transform_gizmo",
]
