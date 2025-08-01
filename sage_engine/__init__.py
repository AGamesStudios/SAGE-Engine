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
    "format",
    "window",
    "graphic",
    "runtime",
    "cursor",
    "input",
    "flow",
    "sprite",
    "animation",
    "plugins",
    "settings",
    "preview",
    "introspection",
    "api",
]

from importlib import import_module

from .logger import logger


def auto_setup() -> None:
    """Initialize engine subsystems (placeholder)."""
    pass


auto_setup()


def __getattr__(name):
    if name in __all__:
        return import_module(f"sage_engine.{name}")
    raise AttributeError(name)

