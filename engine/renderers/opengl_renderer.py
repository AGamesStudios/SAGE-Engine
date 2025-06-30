from __future__ import annotations

from dataclasses import dataclass
from typing import Optional
import math
import ctypes

from PyQt6.QtOpenGLWidgets import QOpenGLWidget
from PyQt6.QtGui import QSurfaceFormat
from OpenGL.GL import (
    glEnable, glBlendFunc, glClearColor, glClear, glPushMatrix, glPopMatrix,
    glTranslatef, glRotatef, glScalef, glBegin, glEnd, glVertex2f, glColor4f,
    glTexCoord2f, glBindTexture, glTexParameteri, glTexImage2D, glGenTextures,
    glLineWidth, glBufferSubData,
    glGetUniformLocation, glUniform4f, glUseProgram, glBindBuffer,
    glBindVertexArray, glDrawArrays,
    GL_BLEND, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_COLOR_BUFFER_BIT,
    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER, GL_LINEAR, GL_NEAREST,
    GL_QUADS, GL_LINES, GL_LINE_LOOP, GL_TRIANGLES, GL_RGBA, GL_UNSIGNED_BYTE,
    GL_MULTISAMPLE, GL_LINE_SMOOTH, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
    GL_FLOAT, GL_TRIANGLE_FAN, GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
)
from OpenGL.GL.shaders import compileProgram, compileShader
from .shader import Shader
from PIL import Image

from engine.core.camera import Camera
from engine.core.game_object import GameObject
from engine import units
from engine.log import logger


class GLWidget(QOpenGLWidget):
    def __init__(self, parent=None):
        fmt = QSurfaceFormat()
        fmt.setSamples(4)
        super().__init__(parent)
        self.setFormat(fmt)
        try:
            from PyQt6.QtCore import Qt
            self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)
        except Exception:
            pass
        self.renderer: Optional['OpenGLRenderer'] = None

    def initializeGL(self):
        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glEnable(GL_MULTISAMPLE)
        glEnable(GL_LINE_SMOOTH)
        glEnable(GL_TEXTURE_2D)
        if self.renderer:
            self.renderer.init_gl()

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
    background: tuple[int, int, int] = (0, 0, 0)

    def create_widget(self) -> GLWidget:
        """Return the :class:`GLWidget` used for rendering."""
        return GLWidget()

    def init_gl(self) -> None:
        """Initialize OpenGL resources and set up the viewport."""
        self.setup_view()
        if self._program is not None:
            return
        vert = """
            #version 120
            attribute vec2 pos;
            attribute vec2 uv;
            varying vec2 v_uv;
            void main() {
                gl_Position = vec4(pos, 0.0, 1.0);
                v_uv = uv;
            }
        """
        frag = """
            #version 120
            varying vec2 v_uv;
            uniform sampler2D tex;
            uniform vec4 color;
            void main() {
                vec4 c = color;
                float m = max(max(c.r, c.g), max(c.b, c.a));
                if (m > 1.0) {
                    c /= 255.0;
                }
                gl_FragColor = texture2D(tex, v_uv) * c;
            }
        """
        from .shader import Shader
        self._sprite_shader = Shader(vert, frag)
        self._program = self._sprite_shader.compile()
        from OpenGL.GL import (
            glGenVertexArrays, glBindVertexArray, glGenBuffers, glBindBuffer,
            glBufferData, GL_ARRAY_BUFFER, GL_STATIC_DRAW, glGetAttribLocation,
            glEnableVertexAttribArray, glVertexAttribPointer, GL_FLOAT,
        )
        self._vao = glGenVertexArrays(1)
        glBindVertexArray(self._vao)
        self._vbo = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self._vbo)
        data = (
            ctypes.c_float * 16
        )(
            -0.5, -0.5, 0.0, 0.0,
            0.5, -0.5, 1.0, 0.0,
            0.5, 0.5, 1.0, 1.0,
            -0.5, 0.5, 0.0, 1.0,
        )
        glBufferData(GL_ARRAY_BUFFER, ctypes.sizeof(data), data, GL_STATIC_DRAW)
        loc_pos = glGetAttribLocation(self._program, "pos")
        loc_uv = glGetAttribLocation(self._program, "uv")
        glEnableVertexAttribArray(loc_pos)
        glVertexAttribPointer(loc_pos, 2, GL_FLOAT, False, 16, ctypes.c_void_p(0))
        glEnableVertexAttribArray(loc_uv)
        glVertexAttribPointer(loc_uv, 2, GL_FLOAT, False, 16, ctypes.c_void_p(8))
        glBindBuffer(GL_ARRAY_BUFFER, 0)
        glBindVertexArray(0)

        # full screen quad for post processing
        self._quad_vao = glGenVertexArrays(1)
        glBindVertexArray(self._quad_vao)
        self._quad_vbo = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self._quad_vbo)
        quad_data = (ctypes.c_float * 8)(-1.0, -1.0, 1.0, -1.0, 1.0, 1.0, -1.0, 1.0)
        glBufferData(GL_ARRAY_BUFFER, ctypes.sizeof(quad_data), quad_data, GL_STATIC_DRAW)
        glBindBuffer(GL_ARRAY_BUFFER, 0)
        glBindVertexArray(0)

    def __post_init__(self):
        if self.widget is None:
            self.widget = self.create_widget()
        self.widget.renderer = self
        self.widget.resize(self.width, self.height)
        self._should_close = False
        self.textures: dict[tuple[int, bool], int] = {}
        self._blank_texture: int | None = None
        self._blank_nearest_texture: int | None = None
        self._icon_cache: dict[str, int] = {}
        self._scene = None
        self._camera = None
        self._draw_gizmos = True
        self._selected_obj = None
        self._hover_axis: str | None = None
        self._drag_axis: str | None = None
        self._cursor_pos: tuple[float, float] | None = None
        self._transform_mode: str = 'pan'
        self._local_coords: bool = False
        # apply sprite effects during rendering
        self.apply_effects: bool = True
        self.show_axes = True
        self.show_grid = False
        self.grid_size = 1.0
        self.grid_color = (0.3, 0.3, 0.3, 1.0)
        self.keep_aspect = bool(self.keep_aspect)
        self.background = tuple(self.background)
        self._program = None
        self._sprite_shader: Shader | None = None
        self._vao = None
        self._vbo = None
        self._quad_vao = None
        self._quad_vbo = None
        self._post_tex = None
        self._post_fbo = None

    def set_window_size(self, width: int, height: int):
        if self.widget:
            self.widget.resize(width, height)
        self.width = width
        self.height = height
        # only update the GL projection if a valid context exists
        ctx = self.widget.context() if self.widget else None
        if ctx and ctx.isValid():
            self.setup_view()

    def should_close(self) -> bool:
        return self._should_close

    def clear(self, color=(0, 0, 0)):
        glClearColor(color[0]/255.0, color[1]/255.0, color[2]/255.0, 1.0)
        glClear(GL_COLOR_BUFFER_BIT)

    def units_y_up(self) -> bool:
        """Return the engine's current Y-axis orientation."""
        return units.Y_UP

    def _capture_screen(self) -> int:
        """Copy the current frame buffer to ``_post_tex`` and return the texture id."""
        from OpenGL.GL import (
            glBindTexture, glCopyTexImage2D, glGenTextures,
            GL_TEXTURE_2D, GL_RGBA
        )
        if self._post_tex is None:
            self._post_tex = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self._post_tex)
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, self.width, self.height, 0)
        glBindTexture(GL_TEXTURE_2D, 0)
        return self._post_tex

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

    def _get_blank_texture(self, smooth: bool = True) -> int:
        """Return a 1x1 white texture used for colored objects."""
        attr = '_blank_texture' if smooth else '_blank_nearest_texture'
        tex = getattr(self, attr, None)
        if tex is None:
            data = b"\xff\xff\xff\xff"
            tex = glGenTextures(1)
            glBindTexture(GL_TEXTURE_2D, tex)
            filt = GL_LINEAR if smooth else GL_NEAREST
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filt)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filt)
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, data
            )
            setattr(self, attr, tex)
        return tex

    def _get_texture(self, obj) -> int:
        if obj.image is None:
            return self._get_blank_texture(obj.smooth)
        key = (id(obj.image), bool(getattr(obj, 'smooth', True)))
        tex = self.textures.get(key)
        if tex:
            return tex
        img = obj.image
        img = img.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
        data = img.tobytes()
        tex_id = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, tex_id)
        filt = GL_LINEAR if getattr(obj, 'smooth', True) else GL_NEAREST
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filt)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filt)
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, data
        )
        self.textures[key] = tex_id
        return tex_id


    def _get_icon_texture(self, name: str) -> int:
        """Load an icon image from ``sage_editor/icons`` and cache it."""
        tex = self._icon_cache.get(name)
        if tex:
            return tex
        from sage_editor.icons import ICON_DIR
        path = ICON_DIR / name
        if not path.is_file():
            logger.warning('Icon %s not found at %s', name, path)
            return self._get_blank_texture()
        try:
            img = Image.open(path).convert('RGBA')
        except Exception:
            logger.exception('Failed to load icon %s', path)
            return self._get_blank_texture()
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
        unit_scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        glBindTexture(GL_TEXTURE_2D, tex)
        glColor4f(1.0, 1.0, 1.0, 1.0)
        glPushMatrix()
        glTranslatef(x * unit_scale, y * unit_scale * sign, 0)
        inv_zoom = 1.0 / zoom if zoom else 1.0
        half = size / 2.0 * inv_zoom
        glBegin(GL_QUADS)
        glTexCoord2f(0.0, 0.0); glVertex2f(-half, -half)
        glTexCoord2f(1.0, 0.0); glVertex2f( half, -half)
        glTexCoord2f(1.0, 1.0); glVertex2f( half,  half)
        glTexCoord2f(0.0, 1.0); glVertex2f(-half,  half)
        glEnd()
        glPopMatrix()

    def _draw_object(
        self,
        obj: GameObject,
        camera: Camera | None,
        cam_shader: Shader | None = None,
    ) -> None:
        if self._program is None:
            return
        from OpenGL.GL import (
            glUseProgram, glGetUniformLocation, glUniform4f, glBindBuffer,
            glBufferSubData, GL_ARRAY_BUFFER, glBindVertexArray, glDrawArrays,
            GL_TRIANGLE_FAN, glBindTexture
        )
        custom_shader = obj.get_shader() if hasattr(obj, "get_shader") else None
        if custom_shader:
            shader = custom_shader
            uniforms = obj.shader_uniforms
        elif cam_shader:
            shader = cam_shader
            uniforms = getattr(camera, "shader_uniforms", {})
        else:
            shader = None

        if shader:
            shader.use(uniforms)
            program = shader.program
        else:
            program = self._program
            glUseProgram(program)
            loc_color = glGetUniformLocation(program, "color")
            rgba = obj.color or (255, 255, 255, 255)
            scale = 1 / 255.0 if max(rgba) > 1.0 else 1.0
            norm = tuple(c * scale for c in rgba)
            glUniform4f(loc_color, *norm)

        shape = getattr(obj, "shape", None)
        if shape in ("triangle", "circle"):
            if shader:
                Shader.stop()
            else:
                glUseProgram(0)
            self._draw_shape(obj, camera, shape)
            return

        if self.apply_effects:
            for eff in getattr(obj, "effects", []):
                if eff.get("type") == "outline":
                    color = eff.get("color", (255, 128, 0, 255))
                    if isinstance(color, str):
                        try:
                            parts = [int(p) for p in color.split(",")]
                            while len(parts) < 4:
                                parts.append(255)
                            color = tuple(parts[:4])
                        except Exception:
                            color = (255, 128, 0, 255)
                    color = tuple(color)
                    width = float(eff.get("width", 3.0))
                    self._draw_outline(obj, camera, color=color, width=width)
                    break
        unit_scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        zoom = camera.zoom if camera else 1.0
        cam_x = camera.x if camera else 0.0
        cam_y = camera.y if camera else 0.0
        w_size = camera.width if (self.keep_aspect and camera) else self.width
        h_size = camera.height if (self.keep_aspect and camera) else self.height
        ang = math.radians(getattr(obj, 'angle', 0.0))
        cos_a = math.cos(ang)
        sin_a = math.sin(ang)
        scale_mul = obj.render_scale(camera, apply_effects=self.apply_effects)
        sx = -1.0 if getattr(obj, "flip_x", False) else 1.0
        sy = -1.0 if getattr(obj, "flip_y", False) else 1.0
        w = obj.width * obj.scale_x * scale_mul
        h = obj.height * obj.scale_y * scale_mul
        verts = [
            (-w/2 * sx, -h/2 * sy),
            (w/2 * sx, -h/2 * sy),
            (w/2 * sx, h/2 * sy),
            (-w/2 * sx, h/2 * sy),
        ]
        data = []
        obj_x, obj_y = obj.render_position(camera, apply_effects=self.apply_effects)
        for vx, vy in verts:
            rx = vx * cos_a - vy * sin_a
            ry = vx * sin_a + vy * cos_a
            world_x = (rx + obj_x) * unit_scale
            world_y = (ry + obj_y) * unit_scale * sign
            ndc_x = (2.0 * (world_x - cam_x * unit_scale) * zoom) / w_size
            ndc_y = (2.0 * (world_y - cam_y * unit_scale * sign) * zoom) / h_size
            data.extend([ndc_x, ndc_y])
        uvs = obj.texture_coords(camera, apply_effects=self.apply_effects)
        arr = (ctypes.c_float * 16)(
            data[0], data[1], uvs[0], uvs[1],
            data[2], data[3], uvs[2], uvs[3],
            data[4], data[5], uvs[4], uvs[5],
            data[6], data[7], uvs[6], uvs[7],
        )
        glBindBuffer(GL_ARRAY_BUFFER, self._vbo)
        glBufferSubData(GL_ARRAY_BUFFER, 0, ctypes.sizeof(arr), arr)
        glBindTexture(GL_TEXTURE_2D, self._get_texture(obj))
        glBindVertexArray(self._vao)
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4)
        glBindVertexArray(0)
        glBindBuffer(GL_ARRAY_BUFFER, 0)
        if shader:
            Shader.stop()
        else:
            glUseProgram(0)

    def _draw_outline(
        self,
        obj: GameObject,
        camera: Camera | None,
        color: tuple[float, float, float, float] = (1.0, 0.5, 0.0, 1.0),
        width: float = 3.0,
    ) -> None:
        """Draw a bright outline around ``obj`` in world coordinates."""
        from OpenGL.GL import (
            glBindTexture, glColor4f, glLineWidth, glBegin, glEnd, glVertex2f,
            glUseProgram, glDisable, glEnable, GL_LINE_LOOP, GL_TEXTURE_2D
        )
        if obj is None:
            return
        unit_scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        scale_mul = obj.render_scale(camera, apply_effects=self.apply_effects)
        obj_x, obj_y = obj.render_position(
            camera, apply_effects=self.apply_effects
        )
        ang = math.radians(getattr(obj, "angle", 0.0))
        cos_a = math.cos(ang)
        sin_a = math.sin(ang)
        sx = -1.0 if getattr(obj, "flip_x", False) else 1.0
        sy = -1.0 if getattr(obj, "flip_y", False) else 1.0
        w = obj.width * obj.scale_x * scale_mul
        h = obj.height * obj.scale_y * scale_mul
        off_x = (0.5 - getattr(obj, "pivot_x", 0.5)) * w * sx
        off_y = (0.5 - getattr(obj, "pivot_y", 0.5)) * h * sy
        corners = [
            (-w / 2 * sx + off_x, -h / 2 * sy + off_y),
            (w / 2 * sx + off_x, -h / 2 * sy + off_y),
            (w / 2 * sx + off_x, h / 2 * sy + off_y),
            (-w / 2 * sx + off_x, h / 2 * sy + off_y),
        ]
        glUseProgram(0)
        glBindTexture(GL_TEXTURE_2D, 0)
        glDisable(GL_TEXTURE_2D)
        color_scale = 1 / 255.0 if max(color) > 1.0 else 1.0
        norm = tuple(c * color_scale for c in color)
        glColor4f(*norm)
        glLineWidth(width)
        glBegin(GL_LINE_LOOP)
        cam_x = camera.x if camera else 0.0
        cam_y = camera.y if camera else 0.0
        zoom = camera.zoom if camera else 1.0
        w_size = camera.width if (self.keep_aspect and camera) else self.width
        h_size = camera.height if (self.keep_aspect and camera) else self.height
        for cx, cy in corners:
            rx = cx * cos_a - cy * sin_a
            ry = cx * sin_a + cy * cos_a
            world_x = (rx + obj_x) * unit_scale
            world_y = (ry + obj_y) * unit_scale * sign
            ndc_x = (2.0 * (world_x - cam_x * unit_scale) * zoom) / w_size
            ndc_y = (2.0 * (world_y - cam_y * unit_scale * sign) * zoom) / h_size
            glVertex2f(ndc_x, ndc_y)
        glEnd()
        glLineWidth(1.0)
        glEnable(GL_TEXTURE_2D)

    def _draw_shape(
        self,
        obj: GameObject,
        camera: Camera | None,
        shape: str,
    ) -> None:
        from OpenGL.GL import (
            glBindTexture,
            glColor4f,
            glBegin,
            glEnd,
            glVertex2f,
            glUseProgram,
            glDisable,
            glEnable,
            GL_TRIANGLE_FAN,
            GL_TRIANGLES,
            GL_TEXTURE_2D,
        )

        unit_scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        scale_mul = obj.render_scale(camera, apply_effects=self.apply_effects)
        cam_x = camera.x if camera else 0.0
        cam_y = camera.y if camera else 0.0
        zoom = camera.zoom if camera else 1.0
        w_size = camera.width if (self.keep_aspect and camera) else self.width
        h_size = camera.height if (self.keep_aspect and camera) else self.height

        glUseProgram(0)
        glBindTexture(GL_TEXTURE_2D, 0)
        glDisable(GL_TEXTURE_2D)

        rgba = obj.color or (255, 255, 255, 255)
        scale = 1 / 255.0 if max(rgba) > 1.0 else 1.0
        glColor4f(rgba[0] * scale, rgba[1] * scale, rgba[2] * scale, rgba[3] * scale)

        ang = math.radians(getattr(obj, "angle", 0.0))
        cos_a = math.cos(ang)
        sin_a = math.sin(ang)
        sx = -1.0 if getattr(obj, "flip_x", False) else 1.0
        sy = -1.0 if getattr(obj, "flip_y", False) else 1.0
        w = obj.width * obj.scale_x * scale_mul
        h = obj.height * obj.scale_y * scale_mul
        off_x = (0.5 - getattr(obj, "pivot_x", 0.5)) * w * sx
        off_y = (0.5 - getattr(obj, "pivot_y", 0.5)) * h * sy

        if shape == "triangle":
            verts = [
                (-w / 2 * sx + off_x, -h / 2 * sy + off_y),
                (w / 2 * sx + off_x, -h / 2 * sy + off_y),
                (0.0 + off_x, h / 2 * sy + off_y),
            ]
            mode = GL_TRIANGLES
        elif shape == "square":
            verts = [
                (-w / 2 * sx + off_x, -h / 2 * sy + off_y),
                (w / 2 * sx + off_x, -h / 2 * sy + off_y),
                (w / 2 * sx + off_x, h / 2 * sy + off_y),
                (-w / 2 * sx + off_x, h / 2 * sy + off_y),
            ]
            mode = GL_TRIANGLE_FAN
        else:
            verts = [(off_x, off_y)]
            r = max(w, h) / 2
            steps = 32
            for i in range(steps + 1):
                ang2 = 2 * math.pi * i / steps
                x = math.cos(ang2) * r * sx + off_x
                y = math.sin(ang2) * r * sy + off_y
                verts.append((x, y))
            mode = GL_TRIANGLE_FAN

        obj_x, obj_y = obj.render_position(camera, apply_effects=self.apply_effects)
        glBegin(mode)
        for vx, vy in verts:
            vx *= sx
            vy *= sy
            rx = vx * cos_a - vy * sin_a
            ry = vx * sin_a + vy * cos_a
            world_x = (rx + obj_x) * unit_scale
            world_y = (ry + obj_y) * unit_scale * sign
            ndc_x = (2.0 * (world_x - cam_x * unit_scale) * zoom) / w_size
            ndc_y = (2.0 * (world_y - cam_y * unit_scale * sign) * zoom) / h_size
            glVertex2f(ndc_x, ndc_y)
        glEnd()
        glEnable(GL_TEXTURE_2D)

    def _draw_frustum(
        self,
        cam: Camera,
        color: tuple[float, float, float, float] = (1.0, 1.0, 0.0, 1.0),
        width: float = 1.0,
    ) -> None:
        """Draw a rectangle representing the camera's view."""
        left, bottom, w, h = cam.view_rect()
        sign = 1.0 if units.Y_UP else -1.0
        glBindTexture(GL_TEXTURE_2D, 0)
        glColor4f(*color)
        glLineWidth(width)
        glBegin(GL_LINE_LOOP)
        glVertex2f(left, bottom * sign)
        glVertex2f(left + w, bottom * sign)
        glVertex2f(left + w, (bottom + h) * sign)
        glVertex2f(left, (bottom + h) * sign)
        glEnd()
        glLineWidth(1.0)

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

    def _draw_grid(self, camera: Camera | None):
        """Draw a simple grid relative to the camera."""
        if not camera:
            zoom = 1.0
            cam_x = cam_y = 0.0
        else:
            zoom = camera.zoom
            cam_x = camera.x
            cam_y = camera.y
        spacing = self.grid_size
        if spacing <= 0:
            return
        scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        half_w = self.width / 2 / (scale * zoom)
        half_h = self.height / 2 / (scale * zoom)
        start_x = math.floor((cam_x - half_w) / spacing) * spacing
        end_x = math.ceil((cam_x + half_w) / spacing) * spacing
        start_y = math.floor((cam_y - half_h) / spacing) * spacing
        end_y = math.ceil((cam_y + half_h) / spacing) * spacing
        glBindTexture(GL_TEXTURE_2D, 0)
        glColor4f(*self.grid_color)
        glBegin(GL_LINES)
        x = start_x
        while x <= end_x:
            glVertex2f(x * scale, start_y * scale * sign)
            glVertex2f(x * scale, end_y * scale * sign)
            x += spacing
        y = start_y
        while y <= end_y:
            glVertex2f(start_x * scale, y * scale * sign)
            glVertex2f(end_x * scale, y * scale * sign)
            y += spacing
        glEnd()


    def _draw_cursor(self, x: float, y: float, camera: Camera | None):
        """Draw a small cross at the cursor position in world units."""
        if camera is None:
            zoom = 1.0
        else:
            zoom = camera.zoom
        size = 5.0 / zoom
        scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        glBindTexture(GL_TEXTURE_2D, 0)
        glPushMatrix()
        glTranslatef(x * scale, y * scale * sign, 0)
        glColor4f(1.0, 1.0, 1.0, 1.0)
        glBegin(GL_LINES)
        glVertex2f(-size, 0.0)
        glVertex2f(size, 0.0)
        glVertex2f(0.0, -size)
        glVertex2f(0.0, size)
        glEnd()
        glPopMatrix()

    def _draw_gizmo(
        self,
        obj: "GameObject",
        camera: Camera | None,
        hover: str | None = None,
        dragging: str | None = None,
        mode: str = 'move',
        local: bool = False,
    ):
        if obj is None or mode == 'pan':
            return
        scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        zoom = camera.zoom if camera else 1.0
        inv = 1.0 / zoom if zoom else 1.0
        ratio = self.widget.devicePixelRatioF() if self.widget else 1.0
        size = 50 * inv
        head = 10 * inv
        rad = 4 * inv
        sq = 6 * inv
        ring_r = size * 1.2
        ring_w = 4 * ratio
        glBindTexture(GL_TEXTURE_2D, 0)
        glPushMatrix()
        glTranslatef(obj.x * scale, obj.y * scale * sign, 0)
        if local or mode == 'scale':
            glRotatef(getattr(obj, 'angle', 0.0), 0, 0, 1)
        base_w = 6 * ratio
        glLineWidth(base_w)
        if mode == 'move':
            # translation arrows - highlight hovered or dragged axes
            hx = hover in ("x", "xy") or dragging in ("x", "xy")
            color_x = 1.0 if hx else 0.7
            glColor4f(color_x, 0.0, 0.0, 1.0)
            glLineWidth(base_w * (1.5 if hx else 1.0))
            glBegin(GL_LINES)
            glVertex2f(0.0, 0.0)
            glVertex2f(size - head, 0.0)
            glEnd()
            glBegin(GL_TRIANGLES)
            glVertex2f(size, 0.0)
            glVertex2f(size - head, head / 2)
            glVertex2f(size - head, -head / 2)
            glEnd()

            hy = hover in ("y", "xy") or dragging in ("y", "xy")
            color_y = 1.0 if hy else 0.7
            glColor4f(0.0, color_y, 0.0, 1.0)
            glLineWidth(base_w * (1.5 if hy else 1.0))
            glBegin(GL_LINES)
            glVertex2f(0.0, 0.0)
            glVertex2f(0.0, sign * (size - head))
            glEnd()
            glBegin(GL_TRIANGLES)
            if units.Y_UP:
                glVertex2f(0.0, size)
                glVertex2f(-head / 2, size - head)
                glVertex2f(head / 2, size - head)
            else:
                glVertex2f(0.0, -size)
                glVertex2f(-head / 2, -size + head)
                glVertex2f(head / 2, -size + head)
            glEnd()

        elif mode == 'scale':
            # scale arrows using squares with tips extended beyond the line
            sx_hover = hover == 'sx' or dragging == 'sx'
            color_sx = 1.0 if sx_hover else 0.7
            glColor4f(color_sx, 0.0, 0.0, 1.0)
            glLineWidth(base_w * (1.5 if sx_hover else 1.0))
            glBegin(GL_LINES)
            glVertex2f(0.0, 0.0)
            glVertex2f(size, 0.0)
            glEnd()
            sq_size = sq * (1.5 if sx_hover else 1.0)
            glBegin(GL_QUADS)
            glVertex2f(size, -sq_size)
            glVertex2f(size + 2 * sq_size, -sq_size)
            glVertex2f(size + 2 * sq_size, sq_size)
            glVertex2f(size, sq_size)
            glEnd()

            sy_hover = hover == 'sy' or dragging == 'sy'
            color_sy = 1.0 if sy_hover else 0.7
            glColor4f(0.0, color_sy, 0.0, 1.0)
            glLineWidth(base_w * (1.5 if sy_hover else 1.0))
            glBegin(GL_LINES)
            glVertex2f(0.0, 0.0)
            glVertex2f(0.0, size * sign)
            glEnd()
            sq_size = sq * (1.5 if sy_hover else 1.0)
            glBegin(GL_QUADS)
            if units.Y_UP:
                glVertex2f(-sq_size, size)
                glVertex2f(sq_size, size)
                glVertex2f(sq_size, size + 2 * sq_size)
                glVertex2f(-sq_size, size + 2 * sq_size)
            else:
                glVertex2f(-sq_size, -size - 2 * sq_size)
                glVertex2f(sq_size, -size - 2 * sq_size)
                glVertex2f(sq_size, -size)
                glVertex2f(-sq_size, -size)
            glEnd()

        elif mode == 'rotate':
            # rotation ring colored like the Z axis
            rot_hover = hover == 'rot' or dragging == 'rot'
            color_rot = 1.0 if rot_hover else 0.7
            glColor4f(0.0, 0.0, color_rot, 1.0)
            glLineWidth(ring_w * (1.5 if rot_hover else 1.0))
            glBegin(GL_LINE_LOOP)
            for i in range(32):
                ang = (i / 32.0) * math.tau
                glVertex2f(math.cos(ang) * ring_r, math.sin(ang) * ring_r * sign)
            glEnd()
            glLineWidth(base_w)

        # pivot point
        center_hover = hover == 'xy' or dragging == 'xy'
        center_col = 1.0 if center_hover else 0.7
        size = rad * (1.5 if center_hover else 1.0)
        glColor4f(center_col, center_col, center_col, 1.0)
        glBegin(GL_QUADS)
        glVertex2f(-size, -size)
        glVertex2f(size, -size)
        glVertex2f(size, size)
        glVertex2f(-size, size)
        glEnd()

        # visualize the hit area for the active or hovered handle
        active = dragging or hover
        if active:
            glColor4f(1.0, 1.0, 1.0, 0.25)
            handle = 12 * inv
            if active in ('x', 'sx'):
                end_x = size if active == 'x' else size + 2 * sq
                glBegin(GL_LINE_LOOP)
                glVertex2f(0.0, handle)
                glVertex2f(end_x, handle)
                glVertex2f(end_x, -handle)
                glVertex2f(0.0, -handle)
                glEnd()
            elif active in ('y', 'sy'):
                end_y = size if active == 'y' else size + 2 * sq
                if active == 'sy' and not units.Y_UP:
                    end_y = -end_y
                glBegin(GL_LINE_LOOP)
                glVertex2f(-handle, 0.0)
                glVertex2f(-handle, sign * end_y)
                glVertex2f(handle, sign * end_y)
                glVertex2f(handle, 0.0)
                glEnd()
            elif active == 'rot':
                inner = ring_r - ring_w
                outer = ring_r + ring_w
                glBegin(GL_LINE_LOOP)
                for i in range(32):
                    ang = (i / 32.0) * math.tau
                    glVertex2f(math.cos(ang) * inner,
                               math.sin(ang) * inner * sign)
                glEnd()
                glBegin(GL_LINE_LOOP)
                for i in range(32):
                    ang = (i / 32.0) * math.tau
                    glVertex2f(math.cos(ang) * outer,
                               math.sin(ang) * outer * sign)
                glEnd()
            elif active == 'xy':
                glBegin(GL_LINE_LOOP)
                glVertex2f(-handle, -handle)
                glVertex2f(handle, -handle)
                glVertex2f(handle, handle)
                glVertex2f(-handle, handle)
                glEnd()
        glLineWidth(1)
        glPopMatrix()

    def _apply_viewport(self, camera: Camera | None) -> tuple[int, int, int, int]:
        """Set GL viewport respecting the camera aspect ratio.

        Returns the viewport rectangle ``(x, y, width, height)`` so the caller
        can optionally restrict further operations to this area using scissor
        tests.
        """
        from OpenGL.GL import glViewport
        w = self.widget.width() if self.widget else self.width
        h = self.widget.height() if self.widget else self.height
        if not self.keep_aspect or camera is None:
            glViewport(0, 0, w, h)
            return 0, 0, w, h
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
        return x, y, vp_w, vp_h

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
        if not self.widget:
            return
        from OpenGL.GL import glViewport
        glViewport(0, 0, self.widget.width(), self.widget.height())
        self.clear((0, 0, 0))
        if self._scene:
            self._render_scene(self._scene, self._camera)

    def draw_scene(self, scene, camera: Camera | None = None, gizmos: bool = True,
                   selected: GameObject | None = None,
                   hover: str | None = None, dragging: str | None = None,
                   cursor: tuple[float, float] | None = None,
                   mode: str = 'pan', local: bool = False):
        """Store the scene and camera then schedule a repaint."""
        self._scene = scene
        self._camera = camera
        self._draw_gizmos = gizmos
        self._selected_obj = selected
        self._hover_axis = hover
        self._drag_axis = dragging
        self._cursor_pos = cursor
        self._transform_mode = mode
        self._local_coords = local
        self.widget.update()

    def _render_scene(self, scene, camera: Camera | None):
        x, y, vp_w, vp_h = self._apply_viewport(camera)
        from OpenGL.GL import glEnable, glDisable, glScissor, GL_SCISSOR_TEST
        glEnable(GL_SCISSOR_TEST)
        glScissor(x, y, vp_w, vp_h)
        self.clear(self.background)
        glDisable(GL_SCISSOR_TEST)
        self._apply_projection(camera)
        cam_shader = camera.get_shader() if hasattr(camera, "get_shader") else None
        glPushMatrix()
        try:
            scale = units.UNITS_PER_METER
            sign = 1.0 if units.Y_UP else -1.0
            if camera:
                glScalef(camera.zoom, camera.zoom, 1.0)
                glTranslatef(-camera.x * scale,
                             -camera.y * scale * sign,
                             0)
            if cam_shader:
                cam_shader.use(getattr(camera, "shader_uniforms", {}))
            scene.sort_objects()
            for obj in scene.objects:
                if isinstance(obj, Camera):
                    if self._draw_gizmos:
                        color = (
                            1.0,
                            0.5,
                            0.0,
                            1.0,
                        ) if obj is self._selected_obj else (1.0, 1.0, 0.0, 1.0)
                        width = 3.0 if obj is self._selected_obj else 1.0
                        self._draw_frustum(obj, color=color, width=width)
                        tex = self._get_icon_texture('camera.png')
                        self._draw_icon(
                            obj.x, obj.y, tex, camera.zoom if camera else 1.0
                        )
                    if obj is self._selected_obj:
                        self._draw_gizmo(
                            obj,
                            camera,
                            self._hover_axis,
                            self._drag_axis,
                            mode=self._transform_mode,
                            local=self._local_coords,
                        )
                    continue
                self._draw_object(obj, camera, cam_shader)
                if obj is self._selected_obj:
                    self._draw_outline(obj, camera)
                if self._draw_gizmos:
                    tex_icon = self._get_icon_texture('object.png')
                    self._draw_icon(
                        obj.x, obj.y, tex_icon, camera.zoom if camera else 1.0
                    )
            if (
                self._draw_gizmos
                and self._selected_obj
                and not isinstance(self._selected_obj, Camera)
            ):
                self._draw_gizmo(
                    self._selected_obj,
                    camera,
                    self._hover_axis,
                    self._drag_axis,
                    mode=self._transform_mode,
                    local=self._local_coords,
                )
            if self.show_grid:
                self._draw_grid(camera)
            if self.show_axes:
                self._draw_origin(50 * scale)
            if self._cursor_pos is not None:
                self._draw_cursor(
                    self._cursor_pos[0], self._cursor_pos[1], camera
                )
        finally:
            if cam_shader:
                Shader.stop()
            glPopMatrix()

        if camera and getattr(camera, "post_effects", None):
            tex = self._capture_screen()
            self.clear(self.background)
            from engine.core.post_effects import get_post_effect
            for eff in camera.post_effects:
                handler = get_post_effect(eff.get("type"))
                if handler:
                    handler.apply(self, tex, self.width, self.height, camera, eff)

    def present(self):
        self.widget.update()

    def close(self):
        self._should_close = True
        if self.widget:
            try:
                from OpenGL.GL import glDeleteTextures
                tex_ids = list(self.textures.values())
                if self._blank_texture:
                    tex_ids.append(self._blank_texture)
                if self._blank_nearest_texture:
                    tex_ids.append(self._blank_nearest_texture)
                tex_ids.extend(self._icon_cache.values())
                if tex_ids:
                    arr = (ctypes.c_uint * len(tex_ids))(*tex_ids)
                    self.widget.makeCurrent()
                    glDeleteTextures(len(tex_ids), arr)
                    self.widget.doneCurrent()
            except Exception:
                logger.exception("Failed to delete GL textures")
            self.widget.close()
