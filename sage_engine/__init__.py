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
    "api",
]

from importlib import import_module

from .logger import logger


def auto_setup() -> None:
    """Initialize native libraries if available."""
    try:
        from .render import rustbridge
        rustbridge._load_lib()
    except Exception as e:
        logger.warning(f"SAGE Engine fallback to software renderer: {e}")


auto_setup()


def __getattr__(name):
    if name in __all__:
        return import_module(f"sage_engine.{name}")
    raise AttributeError(name)

