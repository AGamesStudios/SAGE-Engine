from __future__ import annotations

from .model import Color
from .parser import parse_color
from .blend import blend
from .gradient import Gradient
from .theme import load_theme, Theme
from sage_engine.core import register, expose


class ColorSystem:
    def parse(self, value):
        return parse_color(value)

    def blend(self, bg, fg):
        return blend(bg, fg)

    def gradient(self, stops):
        return Gradient(stops)

    def load_theme(self, path):
        return load_theme(path)


color_system = ColorSystem()


def init(_cfg: dict | None = None):
    expose("color", color_system)


register("boot", init)

__all__ = [
    "Color",
    "color_system",
    "parse_color",
    "blend",
    "Gradient",
    "load_theme",
    "Theme",
]
