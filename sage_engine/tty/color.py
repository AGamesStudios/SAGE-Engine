"""ANSI color utilities for TTY mode."""

from __future__ import annotations

from sage_engine.color import Color, parse_color

RESET = "\033[0m"


def _to_rgb(value: Color | str) -> tuple[int, int, int]:
    col = parse_color(value)
    return col.r, col.g, col.b


def fg(value: Color | str) -> str:
    r, g, b = _to_rgb(value)
    return f"\033[38;2;{r};{g};{b}m"


def bg(value: Color | str) -> str:
    r, g, b = _to_rgb(value)
    return f"\033[48;2;{r};{g};{b}m"


def style(bold: bool) -> str:
    return "\033[1m" if bold else ""
