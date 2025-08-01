"""Post-processing styles for the software renderer."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Callable

StyleFunc = Callable[[bytearray, int, int], None]


_styles: dict[str, StyleFunc] = {}


@dataclass(slots=True)
class WidgetStyle:
    """Basic style for GUI widgets."""

    bg_color: tuple[int, int, int, int] = (40, 40, 40, 255)
    fg_color: tuple[int, int, int, int] = (255, 255, 255, 255)
    padding: int = 4
    border_color: tuple[int, int, int, int] = (0, 0, 0, 255)
    border_width: int = 0


def register(name: str, func: StyleFunc) -> None:
    _styles[name] = func


def apply(name: str, buffer: bytearray, width: int, height: int) -> None:
    func = _styles.get(name)
    if func:
        func(buffer, width, height)


def list_styles() -> list[str]:
    return list(_styles.keys())


def default_style(buf: bytearray, width: int, height: int) -> None:
    pass


def mono_dark(buf: bytearray, width: int, height: int) -> None:
    for y in range(height):
        for x in range(width):
            off = y * width * 4 + x * 4
            r = buf[off + 2]
            g = buf[off + 1]
            b = buf[off]
            lum = (r * 3 + g * 4 + b) // 8
            buf[off:off+3] = bytes((0, lum, lum))


register("default", default_style)
register("mono-dark", mono_dark)

