from .glwidget import GLWidget
from .sdlwidget import SDLWidget
from engine.renderers.opengl.glwidget import register_glwidget
from engine.renderers.sdl_widget import register_sdlwidget

register_glwidget(GLWidget)
register_sdlwidget(SDLWidget)
__all__ = ["GLWidget", "SDLWidget"]

