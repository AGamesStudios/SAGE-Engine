from __future__ import annotations

from typing import Sequence

from .model import Color

NAMED_COLORS = {
    "black": Color(0, 0, 0),
    "white": Color(255, 255, 255),
    "red": Color(255, 0, 0),
    "green": Color(0, 255, 0),
    "blue": Color(0, 0, 255),
    "yellow": Color(255, 255, 0),
    "magenta": Color(255, 0, 255),
    "cyan": Color(0, 255, 255),
}


def parse_color(value: str | Sequence[int] | Color) -> Color:
    """Parse a color from hex string, name, tuple or ``Color``."""
    if isinstance(value, Color):
        return value
    if isinstance(value, str):
        value = value.strip()
        if value.startswith("#"):
            hexv = value[1:]
            if len(hexv) == 6:
                r, g, b = int(hexv[0:2], 16), int(hexv[2:4], 16), int(hexv[4:6], 16)
                return Color(r, g, b)
            if len(hexv) == 8:
                r, g, b, a = (
                    int(hexv[0:2], 16),
                    int(hexv[2:4], 16),
                    int(hexv[4:6], 16),
                    int(hexv[6:8], 16),
                )
                return Color(r, g, b, a)
        return NAMED_COLORS.get(value.lower(), Color(0, 0, 0))
    if isinstance(value, Sequence):
        r, g, b = value[0], value[1], value[2]
        a = value[3] if len(value) > 3 else 255
        return Color(r, g, b, a)
    raise TypeError(f"Unsupported color value: {value!r}")
