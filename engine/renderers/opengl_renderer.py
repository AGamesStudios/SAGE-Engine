from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

from PyQt6.QtOpenGLWidgets import QOpenGLWidget
from OpenGL.GL import *

from engine.core.camera import Camera
from engine import units


class GLWidget(QOpenGLWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.renderer: Optional['OpenGLRenderer'] = None

    def initializeGL(self):
        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

    def paintGL(self):
        if self.renderer:
            self.renderer._paint()


@dataclass
class OpenGLRenderer:
    """Renderer using PyOpenGL and QOpenGLWidget."""

    width: int = 640
    height: int = 480
    title: str = "SAGE 2D"
    widget: Optional[GLWidget] = None

    def __post_init__(self):
        if self.widget is None:
            self.widget = GLWidget()
        self.widget.renderer = self
        self.widget.resize(self.width, self.height)
        self._should_close = False
        self.textures: dict[str, int] = {}

    def set_window_size(self, width: int, height: int):
        if self.widget:
            self.widget.resize(width, height)
        self.width = width
        self.height = height

    def should_close(self) -> bool:
        return self._should_close

    def clear(self, color=(0, 0, 0)):
        glClearColor(color[0]/255.0, color[1]/255.0, color[2]/255.0, 1.0)
        glClear(GL_COLOR_BUFFER_BIT)

    def _paint(self):
        # called from GLWidget.paintGL
        self.clear()
        if self._scene:
            self.draw_scene(self._scene, self._camera)

    def draw_scene(self, scene, camera: Camera | None = None):
        self._scene = scene
        self._camera = camera
        glPushMatrix()
        scale = units.UNITS_PER_METER
        if camera:
            glTranslatef(-camera.x, -camera.y, 0)
        scene._sort_objects()
        for obj in scene.objects:
            if isinstance(obj, Camera):
                continue
            x = obj.x * scale
            y = obj.y * scale
            w = obj.width * obj.scale_x
            h = obj.height * obj.scale_y
            glColor4f(*(c/255.0 for c in (obj.color or (255, 255, 255, 255))))
            glBegin(GL_QUADS)
            glVertex2f(x - w/2, y - h/2)
            glVertex2f(x + w/2, y - h/2)
            glVertex2f(x + w/2, y + h/2)
            glVertex2f(x - w/2, y + h/2)
            glEnd()
        glPopMatrix()
        self.widget.update()

    def present(self):
        self.widget.update()

    def close(self):
        self._should_close = True
        if self.widget:
            self.widget.close()
