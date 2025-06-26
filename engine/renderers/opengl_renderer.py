from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

from PyQt6.QtOpenGLWidgets import QOpenGLWidget
from OpenGL.GL import (
    glEnable, glBlendFunc, glClearColor, glClear, glPushMatrix, glPopMatrix,
    glTranslatef, glRotatef, glScalef, glBegin, glEnd, glVertex2f, glColor4f,
    glTexCoord2f, glBindTexture, glTexParameteri, glTexImage2D, glGenTextures,
    GL_BLEND, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_COLOR_BUFFER_BIT,
    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER, GL_LINEAR,
    GL_QUADS, GL_LINES, GL_LINE_LOOP, GL_RGBA, GL_UNSIGNED_BYTE
)
from PIL import Image

from engine.core.camera import Camera
from engine import units
from pathlib import Path


class GLWidget(QOpenGLWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.renderer: Optional['OpenGLRenderer'] = None

    def initializeGL(self):
        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glEnable(GL_TEXTURE_2D)
        if self.renderer:
            self.renderer.setup_view()

    def paintGL(self):
        if self.renderer:
            self.renderer.paint()

    def resizeGL(self, width: int, height: int):
        if self.renderer:
            self.renderer.set_window_size(width, height)


@dataclass
class OpenGLRenderer:
    """Renderer using PyOpenGL and QOpenGLWidget."""

    width: int = 640
    height: int = 480
    title: str = "SAGE 2D"
    widget: Optional[GLWidget] = None
    keep_aspect: bool = True

    def create_widget(self) -> GLWidget:
        """Return the :class:`GLWidget` used for rendering."""
        return GLWidget()

    def __post_init__(self):
        if self.widget is None:
            self.widget = self.create_widget()
        self.widget.renderer = self
        self.widget.resize(self.width, self.height)
        self._should_close = False
        self.textures: dict[int, int] = {}
        self._blank_texture: int | None = None
        self._icon_cache: dict[str, int] = {}
        self._scene = None
        self._camera = None
        self._draw_gizmos = True
        self.keep_aspect = bool(self.keep_aspect)

    def set_window_size(self, width: int, height: int):
        if self.widget:
            self.widget.resize(width, height)
        self.width = width
        self.height = height
        self.setup_view()

    def should_close(self) -> bool:
        return self._should_close

    def clear(self, color=(0, 0, 0)):
        glClearColor(color[0]/255.0, color[1]/255.0, color[2]/255.0, 1.0)
        glClear(GL_COLOR_BUFFER_BIT)

    def setup_view(self):
        from OpenGL.GL import glMatrixMode, glLoadIdentity, glOrtho, GL_PROJECTION, GL_MODELVIEW
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        # center the origin so camera transforms are stable
        glOrtho(
            -self.width / 2,
            self.width / 2,
            -self.height / 2,
            self.height / 2,
            -1,
            1,
        )
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

    def _get_blank_texture(self) -> int:
        """Return a 1x1 white texture used for colored objects."""
        if self._blank_texture is None:
            data = b"\xff\xff\xff\xff"
            self._blank_texture = glGenTextures(1)
            glBindTexture(GL_TEXTURE_2D, self._blank_texture)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, data
            )
        return self._blank_texture

    def _get_texture(self, obj) -> int:
        if obj.image is None:
            return self._get_blank_texture()
        tex = self.textures.get(id(obj.image))
        if tex:
            return tex
        img = obj.image
        img = img.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
        data = img.tobytes()
        tex_id = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, tex_id)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, data
        )
        self.textures[id(obj.image)] = tex_id
        return tex_id

    def _get_icon_texture(self, name: str) -> int:
        """Load an icon image from ``sage_editor/icons`` and cache it."""
        tex = self._icon_cache.get(name)
        if tex:
            return tex
        path = Path(__file__).resolve().parent.parent.parent / 'sage_editor' / 'icons' / name
        if not path.is_file():
            return self._get_blank_texture()
        img = Image.open(path).convert('RGBA')
        img = img.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
        data = img.tobytes()
        tex = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, tex)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, data
        )
        self._icon_cache[name] = tex
        return tex

    def _draw_icon(self, x: float, y: float, tex: int, zoom: float, size: float = 32.0):
        """Render a billboard icon at ``(x, y)`` in world units."""
        scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        glBindTexture(GL_TEXTURE_2D, tex)
        glColor4f(1.0, 1.0, 1.0, 1.0)
        glPushMatrix()
        glTranslatef(x * scale, y * scale * sign, 0)
        inv_zoom = 1.0 / zoom if zoom else 1.0
        half = size / 2.0 * inv_zoom
        glBegin(GL_QUADS)
        glTexCoord2f(0.0, 0.0); glVertex2f(-half, -half)
        glTexCoord2f(1.0, 0.0); glVertex2f( half, -half)
        glTexCoord2f(1.0, 1.0); glVertex2f( half,  half)
        glTexCoord2f(0.0, 1.0); glVertex2f(-half,  half)
        glEnd()
        glPopMatrix()

    def _draw_frustum(self, cam: Camera):
        """Draw a rectangle representing the camera's view."""
        left, bottom, w, h = cam.view_rect()
        sign = 1.0 if units.Y_UP else -1.0
        glBindTexture(GL_TEXTURE_2D, 0)
        glColor4f(1.0, 1.0, 0.0, 1.0)
        glBegin(GL_LINE_LOOP)
        glVertex2f(left, bottom * sign)
        glVertex2f(left + w, bottom * sign)
        glVertex2f(left + w, (bottom + h) * sign)
        glVertex2f(left, (bottom + h) * sign)
        glEnd()

    def _draw_origin(self, size: float = 1.0):
        glBindTexture(GL_TEXTURE_2D, 0)
        glBegin(GL_LINES)
        glColor4f(1.0, 0.0, 0.0, 1.0)
        glVertex2f(-size, 0.0)
        glVertex2f(size, 0.0)
        glColor4f(0.0, 1.0, 0.0, 1.0)
        glVertex2f(0.0, -size)
        glVertex2f(0.0, size)
        glEnd()

    def _apply_viewport(self, camera: Camera | None) -> None:
        """Set GL viewport respecting the camera aspect ratio."""
        from OpenGL.GL import glViewport
        w = self.widget.width() if self.widget else self.width
        h = self.widget.height() if self.widget else self.height
        if not self.keep_aspect or camera is None:
            glViewport(0, 0, w, h)
            return
        cam_ratio = camera.width / camera.height if camera.height else 1.0
        win_ratio = w / h if h else cam_ratio
        if cam_ratio > win_ratio:
            vp_w = w
            vp_h = int(w / cam_ratio)
            x = 0
            y = int((h - vp_h) / 2)
        else:
            vp_h = h
            vp_w = int(h * cam_ratio)
            x = int((w - vp_w) / 2)
            y = 0
        glViewport(x, y, vp_w, vp_h)

    def _apply_projection(self, camera: Camera | None) -> None:
        from OpenGL.GL import glMatrixMode, glLoadIdentity, glOrtho, GL_PROJECTION, GL_MODELVIEW
        w = (camera.width if (self.keep_aspect and camera) else self.width)
        h = (camera.height if (self.keep_aspect and camera) else self.height)
        sign = 1.0 if units.Y_UP else -1.0
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        glOrtho(-w / 2, w / 2, -h / 2 * sign, h / 2 * sign, -1, 1)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

    def paint(self):
        # called from GLWidget.paintGL
        self.clear()
        if self._scene:
            self._render_scene(self._scene, self._camera)

    def draw_scene(self, scene, camera: Camera | None = None, gizmos: bool = True):
        """Store the scene and camera then schedule a repaint."""
        self._scene = scene
        self._camera = camera
        self._draw_gizmos = gizmos
        self.widget.update()

    def _render_scene(self, scene, camera: Camera | None):
        self._apply_viewport(camera)
        self._apply_projection(camera)
        glPushMatrix()
        scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        if camera:
            glTranslatef(-camera.x * scale,
                         -camera.y * scale * sign,
                         0)
            glScalef(camera.zoom, camera.zoom, 1.0)
        scene.sort_objects()
        for obj in scene.objects:
            if isinstance(obj, Camera):
                if self._draw_gizmos:
                    self._draw_frustum(obj)
                    tex = self._get_icon_texture('camera.png')
                    self._draw_icon(
                        obj.x, obj.y, tex, camera.zoom if camera else 1.0
                    )
                continue
            tex = self._get_texture(obj)
            glBindTexture(GL_TEXTURE_2D, tex)
            glPushMatrix()
            glTranslatef(obj.x * scale, obj.y * scale, 0)
            glRotatef(obj.angle, 0, 0, 1)
            glScalef(obj.scale_x * scale, obj.scale_y * scale, 1)
            w = obj.width
            h = obj.height
            glColor4f(*(c/255.0 for c in (obj.color or (255, 255, 255, 255))))
            glBegin(GL_QUADS)
            glTexCoord2f(0.0, 0.0); glVertex2f(-w/2, -h/2)
            glTexCoord2f(1.0, 0.0); glVertex2f( w/2, -h/2)
            glTexCoord2f(1.0, 1.0); glVertex2f( w/2,  h/2)
            glTexCoord2f(0.0, 1.0); glVertex2f(-w/2,  h/2)
            glEnd()
            glPopMatrix()
            if self._draw_gizmos:
                tex_icon = self._get_icon_texture('object.png')
                self._draw_icon(
                    obj.x, obj.y, tex_icon, camera.zoom if camera else 1.0
                )
        if self._draw_gizmos:
            self._draw_origin(50 * scale)
        glPopMatrix()

    def present(self):
        self.widget.update()

    def close(self):
        self._should_close = True
        if self.widget:
            self.widget.close()
