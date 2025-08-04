from __future__ import annotations

from .buffer import TTYBuffer


def draw_text(
    buffer: TTYBuffer,
    x: int,
    y: int,
    text: str,
    fg: str = "white",
    bg: str = "black",
    bold: bool = False,
) -> None:
    for i, ch in enumerate(text):
        buffer.set_cell(x + i, y, ch, fg, bg, bold)


def draw_rect(
    buffer: TTYBuffer,
    x: int,
    y: int,
    w: int,
    h: int,
    char: str = "#",
    fg: str = "white",
    bg: str = "black",
) -> None:
    for yy in range(y, y + h):
        for xx in range(x, x + w):
            buffer.set_cell(xx, yy, char, fg, bg)


def clear(buffer: TTYBuffer) -> None:
    buffer.clear()
