"""OpenGL render backend using moderngl."""

from __future__ import annotations
from typing import Sequence

try:  # pragma: no cover - optional dependency
    import moderngl
except Exception:  # pragma: no cover - optional
    moderngl = None

try:  # pragma: no cover - optional dependency
    import glfw  # type: ignore
except Exception:  # pragma: no cover - optional
    glfw = None

import numpy as np

from sage_engine.render.base import NDArray, RenderBackend


class OpenGLBackend(RenderBackend):
    """Basic OpenGL backend using a single instanced draw call."""

    def __init__(self) -> None:
        if moderngl is None:
            raise RuntimeError("moderngl not installed")
        self.ctx: moderngl.Context | None = None
        self.window = None
        self.prog = None
        self.line_prog = None
        self.vbo = None
        self.instance_buffer = None
        self.vao = None
        self.draw_calls = 0

    def _create_context(self, width: int, height: int) -> None:
        if glfw is not None and glfw.init():  # pragma: no cover - window creation
            glfw.window_hint(glfw.VISIBLE, glfw.FALSE)
            self.window = glfw.create_window(width, height, b"SAGE", None, None)
            glfw.make_context_current(self.window)
            self.ctx = moderngl.create_context()
        else:
            self.ctx = moderngl.create_standalone_context()
            self.window = None

    def create_device(self, width: int, height: int) -> None:
        self._create_context(width, height)
        assert self.ctx is not None
        self.ctx.enable(moderngl.BLEND)
        self.ctx.blend_func = moderngl.SRC_ALPHA, moderngl.ONE_MINUS_SRC_ALPHA

        quad = np.array([
            -0.5,
            -0.5,
            0.5,
            -0.5,
            -0.5,
            0.5,
            0.5,
            0.5,
        ], dtype="f4")
        self.vbo = self.ctx.buffer(quad.tobytes())
        self.instance_buffer = self.ctx.buffer(reserve=16 * 4096)

        vert_src = """
        #version 330
        in vec2 in_vert;
        in vec2 in_pos;
        in float in_rot;
        in float in_tex;
        void main() {
            mat2 r = mat2(cos(in_rot), -sin(in_rot), sin(in_rot), cos(in_rot));
            gl_Position = vec4(r * in_vert + in_pos, 0.0, 1.0);
        }
        """
        frag_src = """
        #version 330
        out vec4 color;
        void main() {
            color = vec4(1.0);
        }
        """
        self.prog = self.ctx.program(vertex_shader=vert_src, fragment_shader=frag_src)
        self.vao = self.ctx.vertex_array(
            self.prog,
            [(self.vbo, "2f", "in_vert"), (self.instance_buffer, "2f 1f 1f/i", "in_pos", "in_rot", "in_tex")],
        )

        line_vert = """
        #version 330
        in vec2 in_pos;
        void main() {
            gl_Position = vec4(in_pos, 0.0, 1.0);
        }
        """
        line_frag = """
        #version 330
        uniform vec3 u_color;
        out vec4 color;
        void main() {
            color = vec4(u_color, 1.0);
        }
        """
        self.line_prog = self.ctx.program(vertex_shader=line_vert, fragment_shader=line_frag)

        self.resize(width, height)

    def begin_frame(self) -> None:
        self.draw_calls = 0
        if self.ctx:
            self.ctx.clear(0.0, 0.0, 0.0, 1.0)

    def draw_sprites(self, instances: NDArray) -> None:
        if self.ctx is None or self.vao is None:
            return
        arr = np.asarray(instances, dtype="f4")
        if arr.size == 0:
            return
        self.instance_buffer.orphan(arr.nbytes)
        self.instance_buffer.write(arr.tobytes())
        self.vao.render(moderngl.TRIANGLE_STRIP, instances=len(arr))
        self.draw_calls += 1

    def end_frame(self) -> None:
        if self.window is not None:  # pragma: no cover - window sync
            glfw.swap_buffers(self.window)
            glfw.poll_events()

    def resize(self, width: int, height: int) -> None:
        if self.ctx is not None:
            self.ctx.viewport = (0, 0, width, height)
        if self.window is not None:  # pragma: no cover - window resize
            glfw.set_window_size(self.window, width, height)

    def draw_lines(self, vertices: Sequence[float], color: Sequence[float]) -> None:
        if self.ctx is None or self.line_prog is None:
            return
        arr = np.array(vertices, dtype="f4")
        if arr.size == 0:
            return
        vbo = self.ctx.buffer(arr.tobytes())
        vao = self.ctx.vertex_array(self.line_prog, [(vbo, "2f", "in_pos")])
        self.line_prog["u_color"].value = tuple(color[:3])
        vao.render(moderngl.LINES)

__all__ = ["OpenGLBackend"]
