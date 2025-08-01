"""Algorithms for antialiased drawing."""
from __future__ import annotations

from .fx import smart_subpixel_8x


def smart_fxaa(buffer: bytearray, width: int, height: int) -> None:
    """Apply basic FXAA-like smoothing."""
    smart_subpixel_8x(buffer, width, height)
