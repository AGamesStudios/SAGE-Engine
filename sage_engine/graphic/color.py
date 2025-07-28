from __future__ import annotations

from dataclasses import dataclass
from typing import Tuple

@dataclass(frozen=True)
class Color:
    r: int
    g: int
    b: int
    a: int = 255

    def as_tuple(self) -> Tuple[int, int, int, int]:
        return (self.r, self.g, self.b, self.a)


def to_rgba(color) -> Tuple[int, int, int, int]:
    """Convert a color specification to RGBA tuple."""
    if isinstance(color, Color):
        return color.as_tuple()
    if isinstance(color, tuple):
        if len(color) == 3:
            r, g, b = color
            a = 255
        elif len(color) == 4:
            r, g, b, a = color
        else:
            raise ValueError("Invalid color tuple length")
        return (int(r), int(g), int(b), int(a))
    if isinstance(color, str) and color.startswith("#"):
        hexv = color[1:]
        if len(hexv) == 6:
            r = int(hexv[0:2], 16)
            g = int(hexv[2:4], 16)
            b = int(hexv[4:6], 16)
            a = 255
        elif len(hexv) == 8:
            r = int(hexv[0:2], 16)
            g = int(hexv[2:4], 16)
            b = int(hexv[4:6], 16)
            a = int(hexv[6:8], 16)
        else:
            raise ValueError("Invalid hex color length")
        return (r, g, b, a)
    raise TypeError(f"Unsupported color format: {color!r}")
