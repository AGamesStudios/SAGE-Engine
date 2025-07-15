"""Helpers for converting between screen pixels and world units."""

from __future__ import annotations

from . import camera


def world_to_screen(x: float, y: float) -> tuple[float, float]:
    cx, cy, zoom, dpi = camera._x, camera._y, camera._zoom, camera._dpi
    return ((x - cx) * zoom * dpi, (y - cy) * zoom * dpi)


def screen_to_world(px: float, py: float) -> tuple[float, float]:
    cx, cy, zoom, dpi = camera._x, camera._y, camera._zoom, camera._dpi
    return (px / (zoom * dpi) + cx, py / (zoom * dpi) + cy)
