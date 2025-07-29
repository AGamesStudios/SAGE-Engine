"""Basic layout helpers for UI coordinates."""
from __future__ import annotations


def to_screen(x: float, y: float, width: int, height: int) -> tuple[int, int]:
    return int(x * width), int(y * height)

