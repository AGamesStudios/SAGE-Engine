from .glwidget import GLWidget
from engine.renderers.opengl.glwidget import register_glwidget

register_glwidget(GLWidget)

__all__ = ["GLWidget"]

