"""Renderer interface and registry."""

from abc import ABC, abstractmethod
import logging
from importlib import metadata

logger = logging.getLogger(__name__)

RENDERER_REGISTRY: dict[str, type] = {}
_PLUGINS_LOADED = False


def register_renderer(name: str, cls: type) -> None:
    """Register a renderer class under ``name``."""
    RENDERER_REGISTRY[name] = cls


def _load_entry_points() -> None:
    global _PLUGINS_LOADED
    if _PLUGINS_LOADED:
        return
    try:
        eps = metadata.entry_points()
        entries = eps.select(group="sage_engine.renderers") if hasattr(eps, "select") else eps.get("sage_engine.renderers", [])
        for ep in entries:
            try:
                cls = ep.load()
                register_renderer(ep.name, cls)
            except Exception:
                logger.exception("Failed to load renderer %s", ep.name)
    except Exception:
        logger.exception("Error loading renderer entry points")
    _PLUGINS_LOADED = True


def _ensure_default() -> None:
    if "opengl" not in RENDERER_REGISTRY:
        from .opengl_renderer import OpenGLRenderer
        register_renderer("opengl", OpenGLRenderer)
    _load_entry_points()


def get_renderer(name: str) -> type | None:
    """Return the renderer class associated with ``name``."""
    _ensure_default()
    return RENDERER_REGISTRY.get(name)




class Renderer(ABC):
    """Abstract renderer interface."""

    @abstractmethod
    def clear(self, color=(0, 0, 0)):
        """Clear the frame to ``color``."""
        raise NotImplementedError

    @abstractmethod
    def draw_scene(self, scene, camera=None):
        """Draw ``scene`` using ``camera``."""
        raise NotImplementedError

    @abstractmethod
    def present(self):
        """Display the rendered frame."""
        raise NotImplementedError

    @abstractmethod
    def close(self):
        """Shutdown the renderer and free resources."""
        raise NotImplementedError

    def should_close(self) -> bool:
        return False


__all__ = [
    "Renderer",
    "OpenGLRenderer",
    "register_renderer",
    "get_renderer",
    "RENDERER_REGISTRY",
]


def __getattr__(name):
    if name == "OpenGLRenderer":
        _ensure_default()
        return RENDERER_REGISTRY["opengl"]
    raise AttributeError(name)
