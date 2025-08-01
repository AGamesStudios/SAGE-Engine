from .color import Color, to_rgba
from .fx import register as fx_register, apply as fx_apply
from .scene import Scene, Layer, Rect, Group
from .state import GraphicState, config
from .compat import convert

__all__ = [
    "Color",
    "to_rgba",
    "fx_register",
    "fx_apply",
    "Scene",
    "Layer",
    "Rect",
    "Group",
    "GraphicState",
    "config",
    "convert",
]


def __getattr__(name):
    if name in {"api", "style", "backend", "layout", "widget", "event", "manager", "effects", "smoothing", "gradient", "animation", "drawtools"}:
        from importlib import import_module
        return import_module(f"sage_engine.graphic.{name}")
    raise AttributeError(name)

