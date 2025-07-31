"""Placeholder drawing helpers using SAGE Engine's graphic runtime."""

from __future__ import annotations

from typing import Tuple

from sage_engine import gfx


def draw_rect(x: int, y: int, w: int, h: int, color: str) -> None:
    """Fill a rectangle using the engine runtime."""
    gfx.draw_rect(x, y, w, h, color)


def draw_text(text: str, x: int, y: int, color: str) -> None:
    gfx.draw_text(text, x, y, color)
