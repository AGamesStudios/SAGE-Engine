"""Renderer interface and registry."""

try:
    from .opengl_renderer import OpenGLRenderer, GLSettings
except Exception:  # pragma: no cover - optional dependency
    OpenGLRenderer = None
    class GLSettings:
        pass
from .pygame_renderer import PygameRenderer

RENDERER_REGISTRY: dict[str, type] = {}

def register_renderer(name: str, cls: type) -> None:
    """Register a renderer class under ``name``."""
    RENDERER_REGISTRY[name] = cls

def get_renderer(name: str) -> type | None:
    """Return the renderer class associated with ``name``."""
    return RENDERER_REGISTRY.get(name)

register_renderer("pygame", PygameRenderer)
if OpenGLRenderer:
    register_renderer("opengl", OpenGLRenderer)

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
    'Renderer', 'OpenGLRenderer', 'PygameRenderer', 'GLSettings',
    'register_renderer', 'get_renderer', 'RENDERER_REGISTRY'
]
