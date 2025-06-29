"""Renderer interface and registry."""

RENDERER_REGISTRY: dict[str, type] = {}


def register_renderer(name: str, cls: type) -> None:
    """Register a renderer class under ``name``."""
    RENDERER_REGISTRY[name] = cls


def _ensure_default() -> None:
    if "opengl" not in RENDERER_REGISTRY:
        from .opengl_renderer import OpenGLRenderer
        register_renderer("opengl", OpenGLRenderer)


def get_renderer(name: str) -> type | None:
    """Return the renderer class associated with ``name``."""
    _ensure_default()
    return RENDERER_REGISTRY.get(name)




class Renderer:
    """Abstract renderer interface."""

    def clear(self, color=(0, 0, 0)):
        raise NotImplementedError

    def draw_scene(self, scene, camera=None):
        raise NotImplementedError

    def present(self):
        raise NotImplementedError

    def close(self):
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
