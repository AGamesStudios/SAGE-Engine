"""Renderer interface and registry."""

from abc import ABC, abstractmethod
import logging
from importlib import metadata
from typing import Callable

from .shader import Shader
from .null_renderer import NullRenderer


def register_draw_handler(role: str, func: Callable[["Renderer"], None]) -> None:
    """Forward to :func:`engine.renderers.opengl_renderer.register_draw_handler`."""
    from .opengl_renderer import register_draw_handler as _reg
    _reg(role, func)

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
    if "null" not in RENDERER_REGISTRY:
        from .null_renderer import NullRenderer
        register_renderer("null", NullRenderer)
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

    def __init__(self, *args, **kwargs):
        self.post_hooks: list[Callable[["Renderer"], None]] = []

    # hook management ---------------------------------------------------
    def add_post_hook(self, func: Callable[["Renderer"], None]) -> None:
        self.post_hooks.append(func)

    def remove_post_hook(self, func: Callable[["Renderer"], None]) -> None:
        if func in self.post_hooks:
            self.post_hooks.remove(func)

    def run_post_hooks(self) -> None:
        for hook in list(self.post_hooks):
            try:
                hook(self)
            except Exception:
                logger.exception("Post hook error")


__all__ = [
    "Renderer",
    "OpenGLRenderer",
    "NullRenderer",
    "register_renderer",
    "get_renderer",
    "RENDERER_REGISTRY",
    "Shader",
    "register_draw_handler",
]


def __getattr__(name):
    if name == "OpenGLRenderer":
        _ensure_default()
        return RENDERER_REGISTRY["opengl"]
    if name == "NullRenderer":
        _ensure_default()
        if "null" not in RENDERER_REGISTRY:
            from .null_renderer import NullRenderer
            register_renderer("null", NullRenderer)
        return RENDERER_REGISTRY["null"]
    raise AttributeError(name)
