"""SAGE Engine package."""

__all__ = [
    "core",
    "time",
    "timers",
    "events",
    "dag",
    "scene",
    "roles",
    "profiling",
    "tasks",
    "resource",
    "render",
    "settings",
]

from importlib import import_module


def __getattr__(name):
    if name in __all__:
        return import_module(f"sage_engine.{name}")
    raise AttributeError(name)

