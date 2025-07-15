"""OpenGL renderer components."""

from .core import OpenGLRenderer, register_draw_handler, RENDER_HANDLERS
from .glwidget import GLWidget  # compatibility
from . import drawing, gizmos, shaders
from .textures import get_blank_texture, get_texture, unload_texture

__all__ = [
    "OpenGLRenderer",
    "register_draw_handler",
    "RENDER_HANDLERS",
    "GLWidget",
    "drawing",
    "gizmos",
    "shaders",
    "get_blank_texture",
    "get_texture",
    "unload_texture",
]
