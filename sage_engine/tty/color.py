"""ANSI color utilities for TTY mode."""

from __future__ import annotations

COLORS = {
    "black": 30,
    "red": 31,
    "green": 32,
    "yellow": 33,
    "blue": 34,
    "magenta": 35,
    "cyan": 36,
    "white": 37,
}

RESET = "\033[0m"


def fg(color: str) -> str:
    code = COLORS.get(color, 37)
    return f"\033[{code}m"


def bg(color: str) -> str:
    code = COLORS.get(color, 30) + 10
    return f"\033[{code}m"


def style(bold: bool) -> str:
    return "\033[1m" if bold else ""
