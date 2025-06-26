from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

import sdl2
from OpenGL.GL import (
    glEnable, glBlendFunc, glClearColor, glClear, glPushMatrix, glPopMatrix,
    glTranslatef, glRotatef, glScalef, glBegin, glEnd, glVertex2f, glColor4f,
    glTexCoord2f, glBindTexture, glTexParameteri, glTexImage2D, glGenTextures,
    GL_BLEND, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_COLOR_BUFFER_BIT,
    GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_TEXTURE_MAG_FILTER, GL_LINEAR,
    GL_QUADS, GL_LINES, GL_RGBA, GL_UNSIGNED_BYTE
)
from PIL import Image

from engine.core.camera import Camera
from engine import units


@dataclass
class SDL2Renderer:
    """OpenGL renderer using an SDL2 window."""

    width: int = 640
    height: int = 480
    title: str = "SAGE 2D"
    window: Optional[sdl2.SDL_Window] = None

    def __post_init__(self):
        sdl2.SDL_Init(sdl2.SDL_INIT_VIDEO)
        flags = sdl2.SDL_WINDOW_OPENGL | sdl2.SDL_WINDOW_RESIZABLE
        self.window = sdl2.SDL_CreateWindow(
            self.title.encode(),
            sdl2.SDL_WINDOWPOS_CENTERED,
            sdl2.SDL_WINDOWPOS_CENTERED,
            self.width,
            self.height,
            flags,
        )
        sdl2.SDL_GL_SetAttribute(sdl2.SDL_GL_DOUBLEBUFFER, 1)
        self.context = sdl2.SDL_GL_CreateContext(self.window)
        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glEnable(GL_TEXTURE_2D)
        self._should_close = False
        self.textures: dict[int, int] = {}
        self._blank_texture: int | None = None
        self._scene = None
        self._camera = None
        self.set_window_size(self.width, self.height)

    def set_window_size(self, width: int, height: int) -> None:
        self.width = width
        self.height = height
        sdl2.SDL_SetWindowSize(self.window, width, height)
        self.setup_view()

    def should_close(self) -> bool:
        return self._should_close

    def clear(self, color=(0, 0, 0)) -> None:
        glClearColor(color[0] / 255.0, color[1] / 255.0, color[2] / 255.0, 1.0)
        glClear(GL_COLOR_BUFFER_BIT)

    def setup_view(self) -> None:
        from OpenGL.GL import (
            glMatrixMode,
            glLoadIdentity,
            glOrtho,
            GL_PROJECTION,
            GL_MODELVIEW,
        )
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
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
        if self._blank_texture is None:
            data = b"\xff\xff\xff\xff"
            self._blank_texture = glGenTextures(1)
            glBindTexture(GL_TEXTURE_2D, self._blank_texture)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                1,
                1,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                data,
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
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            img.width,
            img.height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            data,
        )
        self.textures[id(obj.image)] = tex_id
        return tex_id

    def _draw_origin(self, size: float = 1.0) -> None:
        glBindTexture(GL_TEXTURE_2D, 0)
        glBegin(GL_LINES)
        glColor4f(1.0, 0.0, 0.0, 1.0)
        glVertex2f(-size, 0.0)
        glVertex2f(size, 0.0)
        glColor4f(0.0, 1.0, 0.0, 1.0)
        glVertex2f(0.0, -size)
        glVertex2f(0.0, size)
        glEnd()

    def paint(self) -> None:
        self.clear()
        if self._scene:
            self._render_scene(self._scene, self._camera)

    def draw_scene(self, scene, camera: Camera | None = None) -> None:
        self._scene = scene
        self._camera = camera

    def _render_scene(self, scene, camera: Camera | None) -> None:
        glPushMatrix()
        scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        if camera:
            glTranslatef(-camera.x * scale, -camera.y * scale * sign, 0)
            glScalef(camera.zoom, camera.zoom, 1.0)
        scene.sort_objects()
        for obj in scene.objects:
            if isinstance(obj, Camera):
                continue
            tex = self._get_texture(obj)
            glBindTexture(GL_TEXTURE_2D, tex)
            glPushMatrix()
            glTranslatef(obj.x * scale, obj.y * scale, 0)
            glRotatef(obj.angle, 0, 0, 1)
            glScalef(obj.scale_x * scale, obj.scale_y * scale, 1)
            w = obj.width
            h = obj.height
            glColor4f(*(c / 255.0 for c in (obj.color or (255, 255, 255, 255))))
            glBegin(GL_QUADS)
            glTexCoord2f(0.0, 0.0)
            glVertex2f(-w / 2, -h / 2)
            glTexCoord2f(1.0, 0.0)
            glVertex2f(w / 2, -h / 2)
            glTexCoord2f(1.0, 1.0)
            glVertex2f(w / 2, h / 2)
            glTexCoord2f(0.0, 1.0)
            glVertex2f(-w / 2, h / 2)
            glEnd()
            glPopMatrix()
        self._draw_origin(50 * scale)
        glPopMatrix()

    def present(self) -> None:
        sdl2.SDL_GL_SwapWindow(self.window)

    def close(self) -> None:
        self._should_close = True
        if self.window:
            sdl2.SDL_GL_DeleteContext(self.context)
            sdl2.SDL_DestroyWindow(self.window)
            sdl2.SDL_Quit()
