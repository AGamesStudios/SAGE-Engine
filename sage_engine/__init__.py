"""SAGE Engine package."""

from importlib import import_module

__all__ = [
    "core",
    "logger",
    "window",
    "render",
    "input",
    "world",
    "gfx",
]


def __getattr__(name: str):
    if name in __all__:
        return import_module(f"sage_engine.{name}")
    raise AttributeError(name)
