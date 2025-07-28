"""SAGE Engine package."""

__all__ = [
    "core",
    "scheduler",
    "events",
    "dag",
    "world",
    "roles",
    "profiling",
    "tasks",
    "logger",
    "resource",
    "render",
    "gfx",
    "effects",
    "objects",
    "window",
    "settings",
    "api",
]

from importlib import import_module


def __getattr__(name):
    if name in __all__:
        return import_module(f"sage_engine.{name}")
    raise AttributeError(name)

