"""Graphic drawing helpers that proxy to the render system."""

from __future__ import annotations

from sage_engine.core import get


def draw_rect(x: int, y: int, w: int, h: int, color: tuple[int, int, int]) -> None:
    renderer = get("render")
    renderer.draw_rect(x, y, w, h, color)


def draw_text(x: int, y: int, text: str, color: tuple[int, int, int]) -> None:
    renderer = get("render")
    renderer.draw_text(x, y, text, color)
