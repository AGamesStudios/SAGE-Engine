# ruff: noqa: F401,F403,F405
from __future__ import annotations

from dataclasses import dataclass

from .. import Renderer
from typing import Optional, Callable, Any
import math
import ctypes

from .glwidget import GLWidget
from .textures import get_blank_texture, get_texture, unload_texture
try:
    import OpenGL.GL as GL  # type: ignore[import-not-found]
except Exception as exc:  # pragma: no cover - optional dependency
    GL: Any = None
    OPENGL_IMPORT_ERROR = exc
else:
    OPENGL_IMPORT_ERROR = None
from ..shader import Shader
from PIL import Image, ImageDraw  # type: ignore[import-not-found]

from sage_engine.core.camera import Camera
from sage_engine.entities.game_object import GameObject
from sage_engine.utils import units
from sage_engine.utils.log import logger
from sage_engine.mesh_utils import Mesh
from sage_engine.entities.tile_map import TileMap
from . import drawing, gizmos, shaders

# role-specific draw callbacks
RENDER_HANDLERS: dict[str, Callable[[OpenGLRenderer, GameObject, Optional[Camera]], None]] = {}


def register_draw_handler(role: str, func: Callable[[OpenGLRenderer, GameObject, Optional[Camera]], None]) -> None:
    """Register a custom draw handler for ``role``."""
    RENDER_HANDLERS[role] = func


@dataclass
class OpenGLRenderer(Renderer):
    """Renderer using PyOpenGL and QOpenGLWidget."""

    width: int = 640
    height: int = 480
    title: str = "SAGE 2D"
    samples: int = 4
    vsync: bool | None = None
    widget: Optional[Any] = None
    keep_aspect: bool = True
    background: tuple[int, int, int] = (0, 0, 0)

    def create_widget(self) -> Any:
        """Return the :class:`GLWidget` used for rendering."""
        from .. import opengl_renderer
        return opengl_renderer.GLWidget(samples=self.samples, vsync=self.vsync)

    def init_gl(self) -> None:
        """Initialize OpenGL resources and set up the viewport."""
        self.setup_view()
        if self._program is not None:
            return
        self._sprite_shader = shaders.create_sprite_shader()
        self._program = self._sprite_shader.compile()
        self._vao, self._vbo = shaders.setup_sprite_vao(self._sprite_shader)

        # full screen quad for post processing
        self._quad_vao = GL.glGenVertexArrays(1)
        GL.glBindVertexArray(self._quad_vao)
        self._quad_vbo = GL.glGenBuffers(1)
        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, self._quad_vbo)
        quad_data = (ctypes.c_float * 8)(-1.0, -1.0, 1.0, -1.0, 1.0, 1.0, -1.0, 1.0)
        GL.glBufferData(GL.GL_ARRAY_BUFFER, ctypes.sizeof(quad_data), quad_data, GL.GL_STATIC_DRAW)
        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, 0)
        GL.glBindVertexArray(0)

    def __post_init__(self):
        super().__init__()
        if GL is None:
            logger.error("OpenGLRenderer unavailable: %s", OPENGL_IMPORT_ERROR)
            raise ImportError(
                "OpenGLRenderer requires PyOpenGL; install it with 'pip install PyOpenGL'"
            ) from OPENGL_IMPORT_ERROR
        if self.widget is None:
            self.widget = self.create_widget()
        widget = self.widget
        assert widget is not None
        widget.renderer = self
        widget.resize(self.width, self.height)
        self._should_close = False
        self.textures: dict[tuple[int, bool], int] = {}
        self._blank_texture: int | None = None
        self._blank_nearest_texture: int | None = None
        self._scene = None
        self._camera = None
        self._draw_gizmos = True
        self._selected_obj = None
        self._hover_axis: str | None = None
        self._drag_axis: str | None = None
        self._cursor_pos: tuple[float, float] | None = None
        self._transform_mode: str = 'pan'
        self._local_coords: bool = False
        self.show_axes = True
        self.show_grid = False
        self.wireframe = False
        self.grid_size = 1.0
        self.grid_color = (0.3, 0.3, 0.3, 1.0)
        self.keep_aspect = bool(self.keep_aspect)
        self.background = tuple(self.background)  # type: ignore[assignment]
        self._program = None
        self._sprite_shader: Shader | None = None
        self._vao = None
        self._vbo = None
        self._quad_vao = None
        self._quad_vbo = None
        self._projection: list[float] | None = None

    def draw_sprites(self, sprites: list[tuple[int, tuple[float, float, float, float]]]) -> None:
        """Draw *sprites* using a trivial immediate mode API."""
        for tex_id, (x, y, w, h) in sprites:
            GL.glBindTexture(GL.GL_TEXTURE_2D, tex_id)
            GL.glBegin(GL.GL_QUADS)
            GL.glTexCoord2f(0.0, 0.0)
            GL.glVertex2f(x, y)
            GL.glTexCoord2f(1.0, 0.0)
            GL.glVertex2f(x + w, y)
            GL.glTexCoord2f(1.0, 1.0)
            GL.glVertex2f(x + w, y + h)
            GL.glTexCoord2f(0.0, 1.0)
            GL.glVertex2f(x, y + h)
            GL.glEnd()

    def set_window_size(self, width: int, height: int):
        if self.widget:
            self.widget.resize(width, height)
        self.width = width
        self.height = height
        if width == 0 or height == 0:
            return
        # only update the GL projection if a valid context exists
        ctx = self.widget.context() if self.widget else None
        if ctx and ctx.isValid():
            self.setup_view()

    def should_close(self) -> bool:
        return self._should_close

    def clear(self, color=(0, 0, 0)):
        GL.glClearColor(color[0] / 255.0, color[1] / 255.0, color[2] / 255.0, 1.0)
        GL.glClear(GL.GL_COLOR_BUFFER_BIT)

    def units_y_up(self) -> bool:
        """Return the engine's current Y-axis orientation."""
        return units.Y_UP


    def grab_image(self) -> Image.Image:
        """Return the current frame buffer as a :class:`PIL.Image.Image`."""
        widget = self.widget
        if widget is None:
            return Image.new("RGBA", (self.width, self.height))
        ctx = widget.context()
        if ctx and ctx.isValid():
            widget.makeCurrent()
            try:
                # draw the latest frame before reading pixels
                self.paint()
                GL.glPixelStorei(GL.GL_PACK_ALIGNMENT, 1)
                data = GL.glReadPixels(
                    0,
                    0,
                    self.width,
                    self.height,
                    GL.GL_RGBA,
                    GL.GL_UNSIGNED_BYTE,
                )
            except Exception as exc:
                # avoid a noisy traceback if the read fails due to lost context
                code = getattr(exc, "err", None)
                msg = getattr(exc, "description", None)
                if msg is None:
                    msg = str(exc)
                if isinstance(msg, bytes):
                    try:
                        msg = msg.decode("utf-8")
                    except Exception:
                        msg = msg.decode("latin-1", "replace")
                if code is not None:
                    logger.error("glReadPixels failed (%s): %s", code, msg)
                else:
                    logger.error("glReadPixels failed: %s", msg)
                data = bytes(self.width * self.height * 4)
            finally:
                widget.doneCurrent()
        else:  # pragma: no cover - invalid context during shutdown
            data = bytes(self.width * self.height * 4)
        img = Image.frombytes("RGBA", (self.width, self.height), data)
        return img.transpose(Image.Transpose.FLIP_TOP_BOTTOM)

    def save_screenshot(self, path: str) -> None:
        """Save the current frame buffer to ``path`` as PNG."""
        self.grab_image().save(path)

    def setup_view(self):
        """Configure the OpenGL projection and store it for later."""
        from sage_engine.core import math2d
        GL.glMatrixMode(GL.GL_PROJECTION)
        GL.glLoadIdentity()
        # center the origin so camera transforms are stable
        GL.glOrtho(
            -self.width / 2,
            self.width / 2,
            -self.height / 2,
            self.height / 2,
            -1,
            1,
        )
        self._projection = math2d.make_ortho(
            -self.width / 2,
            self.width / 2,
            -self.height / 2,
            self.height / 2,
        )
        GL.glMatrixMode(GL.GL_MODELVIEW)
        GL.glLoadIdentity()

    def get_projection(self) -> list[float] | None:
        """Return the current orthographic projection matrix."""
        return self._projection

    # texture utilities -------------------------------------------------

    def _get_blank_texture(self, smooth: bool = True) -> int:
        return get_blank_texture(self, smooth)

    def _get_texture(self, obj) -> int:
        return get_texture(self, obj)

    def unload_texture(self, obj) -> None:
        unload_texture(self, obj)

    def _build_map_texture(self, tilemap: TileMap) -> None:
        w = tilemap.width * tilemap.tile_width
        h = tilemap.height * tilemap.tile_height
        img = Image.new("RGBA", (w, h))
        draw_img = ImageDraw.Draw(img)
        tw = tilemap.tile_width
        th = tilemap.tile_height
        colors = tilemap.metadata.get("colors", {})
        for y in range(tilemap.height):
            for x in range(tilemap.width):
                idx = tilemap.data[y * tilemap.width + x]
                if idx == 0:
                    continue
                rgba = drawing.parse_color(colors.get(str(idx), (200, 200, 200, 255)))
                rect = [x * tw, y * th, x * tw + tw - 1, y * th + th - 1]
                draw_img.rectangle(rect, fill=rgba)
        arr_bytes = img.tobytes()
        arr = (ctypes.c_ubyte * len(arr_bytes)).from_buffer_copy(arr_bytes)
        tex = GL.glGenTextures(1)
        GL.glBindTexture(GL.GL_TEXTURE_2D, tex)
        GL.glTexParameteri(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MIN_FILTER, GL.GL_NEAREST)
        GL.glTexParameteri(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MAG_FILTER, GL.GL_NEAREST)
        GL.glTexImage2D(
            GL.GL_TEXTURE_2D,
            0,
            GL.GL_RGBA,
            w,
            h,
            0,
            GL.GL_RGBA,
            GL.GL_UNSIGNED_BYTE,
            arr,
        )
        tilemap._texture = tex

    def _draw_map(self, tilemap: TileMap) -> None:
        if getattr(tilemap, "_texture", None) is None:
            self._build_map_texture(tilemap)
        w = tilemap.width * tilemap.tile_width
        h = tilemap.height * tilemap.tile_height
        GL.glBindTexture(GL.GL_TEXTURE_2D, tilemap._texture)
        GL.glBegin(GL.GL_QUADS)
        GL.glTexCoord2f(0, 0)
        GL.glVertex2f(0, 0)
        GL.glTexCoord2f(1, 0)
        GL.glVertex2f(w, 0)
        GL.glTexCoord2f(1, 1)
        GL.glVertex2f(w, h)
        GL.glTexCoord2f(0, 1)
        GL.glVertex2f(0, h)
        GL.glEnd()



    def _parse_color(self, value) -> tuple[int, int, int, int]:
        return drawing.parse_color(value)

    def _draw_object(
        self,
        obj: GameObject,
        camera: Optional[Camera],
        cam_shader: Shader | None = None,
    ) -> None:
        if self._program is None:
            return
        shader = None
        handler = RENDER_HANDLERS.get(getattr(obj, "role", ""))
        if handler:
            handler(self, obj, camera)
            return
        if isinstance(obj, TileMap):
            GL.glUseProgram(0)
            self._draw_map(obj)
            return
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
            GL.glUseProgram(program)
            loc_color = GL.glGetUniformLocation(program, "color")
            rgba = self._parse_color(obj.color)
            scale = 1 / 255.0 if max(rgba) > 1.0 else 1.0
            alpha = getattr(obj, 'alpha', 1.0)
            if alpha > 1.0:
                alpha = alpha / 255.0
            norm = (
                rgba[0] * scale,
                rgba[1] * scale,
                rgba[2] * scale,
                min(1.0, rgba[3] * scale * alpha),
            )
            GL.glUniform4f(loc_color, *norm)

        mesh = getattr(obj, "mesh", None)
        if isinstance(mesh, Mesh):
            if shader:
                Shader.stop()
            else:
                GL.glUseProgram(0)
            self._draw_mesh(obj, camera, mesh)
            return

        shape = getattr(obj, "shape", None)
        if isinstance(shape, str):
            shape = shape.strip().lower()
        if shape in ("rectangle", "rect"):
            shape = "square"
        if shape in ("triangle", "circle", "square"):
            if shader:
                Shader.stop()
            else:
                GL.glUseProgram(0)
            self._draw_shape(obj, camera, shape)
            return

        unit_scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        ang = math.radians(getattr(obj, 'angle', 0.0))
        cos_a = math.cos(ang)
        sin_a = math.sin(ang)
        scale_mul = obj.render_scale(camera)
        sx = -1.0 if getattr(obj, "flip_x", False) else 1.0
        sy = -1.0 if getattr(obj, "flip_y", False) else 1.0
        w = obj.width * obj.scale_x * scale_mul
        h = obj.height * obj.scale_y * scale_mul
        px = w * obj.pivot_x
        py = h * obj.pivot_y
        corners = [
            (-px, -py),
            (w - px, -py),
            (w - px, h - py),
            (-px, h - py),
        ]
        data = []
        obj_x, obj_y = obj.render_position(camera)
        for cx, cy in corners:
            vx = cx * sx
            vy = cy * sy
            rx = vx * cos_a - vy * sin_a
            ry = vx * sin_a + vy * cos_a
            world_x = (rx + obj_x) * unit_scale
            world_y = (ry + obj_y) * unit_scale * sign
            data.extend([world_x, world_y])
        uvs = obj.texture_coords(camera)
        arr = (ctypes.c_float * 16)(
            data[0], data[1], uvs[0], uvs[1],
            data[2], data[3], uvs[2], uvs[3],
            data[4], data[5], uvs[4], uvs[5],
            data[6], data[7], uvs[6], uvs[7],
        )
        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, self._vbo)
        GL.glBufferSubData(GL.GL_ARRAY_BUFFER, 0, ctypes.sizeof(arr), arr)
        GL.glBindTexture(GL.GL_TEXTURE_2D, self._get_texture(obj))
        GL.glBindVertexArray(self._vao)
        GL.glDrawArrays(GL.GL_TRIANGLE_FAN, 0, 4)
        GL.glBindVertexArray(0)
        GL.glBindBuffer(GL.GL_ARRAY_BUFFER, 0)
        if shader:
            Shader.stop()
        else:
            GL.glUseProgram(0)

    def _draw_outline(
        self,
        obj: GameObject,
        camera: Optional[Camera],
        color: tuple[float, float, float, float] = (1.0, 0.5, 0.0, 1.0),
        width: float = 3.0,
    ) -> None:
        drawing.draw_outline(self, obj, camera, color=color, width=width)

    def _draw_mesh(
        self,
        obj: GameObject,
        camera: Optional[Camera],
        mesh: Mesh,
    ) -> None:
        if self.wireframe:
            GL.glPolygonMode(GL.GL_FRONT_AND_BACK, GL.GL_LINE)
        drawing.draw_mesh(self, obj, camera, mesh)
        if self.wireframe:
            GL.glPolygonMode(GL.GL_FRONT_AND_BACK, GL.GL_FILL)

    def _draw_shape(
        self,
        obj: GameObject,
        camera: Optional[Camera],
        shape: str,
    ) -> None:
        if self.wireframe:
            GL.glPolygonMode(GL.GL_FRONT_AND_BACK, GL.GL_LINE)
        drawing.draw_shape(self, obj, camera, shape)
        if self.wireframe:
            GL.glPolygonMode(GL.GL_FRONT_AND_BACK, GL.GL_FILL)

    def _draw_frustum(
        self,
        cam: Camera,
        color: tuple[float, float, float, float] = (1.0, 1.0, 0.0, 1.0),
        width: float = 1.0,
    ) -> None:
        drawing.draw_frustum(self, cam, color=color, width=width)

    def _draw_origin(self, camera: Optional[Camera]):
        drawing.draw_origin(self, camera)

    def _draw_grid(self, camera: Optional[Camera]):
        drawing.draw_grid(self, camera)


    def _draw_cursor(self, x: float, y: float, camera: Optional[Camera]):
        drawing.draw_cursor(self, x, y, camera)

    def _draw_basic_gizmos(self, camera: Optional[Camera]) -> None:
        for gizmo in list(self.gizmos):
            gizmos.draw_basic_gizmo(self, gizmo, camera)
        self._advance_gizmos()


    def _draw_gizmo(
        self,
        obj: GameObject,
        camera: Optional[Camera],
        hover: str | None = None,
        dragging: str | None = None,
        mode: str = 'move',
        local: bool = False,
    ):
        gizmos.draw_gizmo(self, obj, camera, hover, dragging, mode, local)

    def _apply_viewport(self, camera: Optional[Camera]) -> tuple[int, int, int, int]:
        """Set GL viewport respecting the camera aspect ratio.

        Returns the viewport rectangle ``(x, y, width, height)`` so the caller
        can optionally restrict further operations to this area using scissor
        tests.
        """
        w = self.widget.width() if self.widget else self.width
        h = self.widget.height() if self.widget else self.height
        if not self.keep_aspect or camera is None:
            GL.glViewport(0, 0, w, h)
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
        GL.glViewport(x, y, vp_w, vp_h)
        return x, y, vp_w, vp_h

    def _apply_projection(self, camera: Optional[Camera]) -> None:
        from sage_engine.core import math2d
        w = (camera.width if (self.keep_aspect and camera) else self.width)
        h = (camera.height if (self.keep_aspect and camera) else self.height)
        sign = 1.0 if units.Y_UP else -1.0
        GL.glMatrixMode(GL.GL_PROJECTION)
        GL.glLoadIdentity()
        GL.glOrtho(-w / 2, w / 2, -h / 2 * sign, h / 2 * sign, -1, 1)
        self._projection = math2d.make_ortho(
            -w / 2, w / 2, -h / 2 * sign, h / 2 * sign
        )
        GL.glMatrixMode(GL.GL_MODELVIEW)
        GL.glLoadIdentity()

    def paint(self):
        """Draw the current scene to the widget."""
        if not self.widget:
            return
        if self.widget.width() == 0 or self.widget.height() == 0:
            return
        ctx = self.widget.context()
        if ctx is None or not ctx.isValid():
            return
        GL.glViewport(0, 0, self.widget.width(), self.widget.height())
        self.clear((0, 0, 0))
        if self._scene:
            self._render_scene(self._scene, self._camera)

    def draw_scene(self, scene, camera: Optional[Camera] = None, gizmos: bool = True,
                   selected: Optional[GameObject] = None,
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
        if self.widget:
            self.widget.update()

    def _render_scene(self, scene, camera: Optional[Camera]):
        x, y, vp_w, vp_h = self._apply_viewport(camera)
        GL.glEnable(GL.GL_SCISSOR_TEST)
        GL.glScissor(x, y, vp_w, vp_h)
        self.clear(self.background)
        GL.glDisable(GL.GL_SCISSOR_TEST)
        self._apply_projection(camera)
        cam_shader = camera.get_shader() if camera and hasattr(camera, "get_shader") else None
        GL.glPushMatrix()
        try:
            scale = units.UNITS_PER_METER
            sign = 1.0 if units.Y_UP else -1.0
            if camera:
                GL.glScalef(camera.zoom, camera.zoom, 1.0)
                GL.glTranslatef(
                    -camera.x * scale,
                    -camera.y * scale * sign,
                    0,
                )
            if cam_shader:
                cam_shader.use(getattr(camera, "shader_uniforms", {}))
            scene.sort_objects()
            for obj in scene.objects:
                if not getattr(obj, 'visible', True):
                    continue
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
                    if obj is self._selected_obj and self._transform_mode != "rect":
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
                    # icons removed; keep placeholder for future gizmos
                    pass
            if (
                self._draw_gizmos
                and self._selected_obj
                and not isinstance(self._selected_obj, Camera)
                and self._transform_mode != "rect"
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
                self._draw_origin(camera)
            if self.gizmos:
                self._draw_basic_gizmos(camera)
            if self._cursor_pos is not None:
                self._draw_cursor(
                    self._cursor_pos[0], self._cursor_pos[1], camera
                )
        finally:
            if cam_shader:
                Shader.stop()
            GL.glPopMatrix()


    def present(self):
        if self.widget:
            self.widget.update()
        self.run_post_hooks()

    def close(self):
        self._should_close = True
        if self.widget:
            try:
                tex_ids = list(self.textures.values())
                if self._blank_texture:
                    tex_ids.append(self._blank_texture)
                if self._blank_nearest_texture:
                    tex_ids.append(self._blank_nearest_texture)
                if tex_ids:
                    arr = (ctypes.c_uint * len(tex_ids))(*tex_ids)
                    ctx = self.widget.context()
                    if ctx and ctx.isValid():
                        self.widget.makeCurrent()
                        GL.glDeleteTextures(len(tex_ids), arr)
                        self.widget.doneCurrent()
            except Exception:
                logger.exception("Failed to delete GL textures")
            self.widget.close()
            self.widget = None
            self._program = None

    def reset(self) -> None:
        self.textures.clear()


__all__ = ["OpenGLRenderer", "register_draw_handler", "RENDER_HANDLERS"]
