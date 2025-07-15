from .glwidget import GLWidget
from .sdlwidget import SDLWidget
from sage_engine.renderers.opengl.glwidget import register_glwidget
from sage_engine.renderers.sdl_widget import register_sdlwidget

register_glwidget(GLWidget)
register_sdlwidget(SDLWidget)
__all__ = ["GLWidget", "SDLWidget"]

