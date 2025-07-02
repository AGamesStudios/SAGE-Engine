"""Compatibility wrapper for the OpenGL renderer."""

from .opengl.core import OpenGLRenderer, register_draw_handler, RENDER_HANDLERS
from .opengl.glwidget import GLWidget
from .opengl.textures import get_blank_texture, get_texture

__all__ = [
    "OpenGLRenderer",
    "register_draw_handler",
    "RENDER_HANDLERS",
    "GLWidget",
    "get_blank_texture",
    "get_texture",
]
