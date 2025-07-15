"""OpenGL render backend using moderngl."""

from __future__ import annotations
from typing import Sequence, Any


try:  # pragma: no cover - optional dependency
    import moderngl
except Exception:  # pragma: no cover - optional
    moderngl = None

try:  # pragma: no cover - optional dependency
    import glfw  # type: ignore
except Exception:  # pragma: no cover - optional
    glfw = None

import numpy as np

from importlib import resources
from dataclasses import dataclass
from sage_engine.render.base import NDArray, RenderBackend
from sage_engine.render.shader import ShaderProgram
from sage_engine.render.material import Material


@dataclass
class _Atlas:
    texture: Any
    next_x: int = 0
    next_y: int = 0
    row_h: int = 0


class OpenGLBackend(RenderBackend):
    """Basic OpenGL backend using a single instanced draw call."""

    def __init__(self) -> None:
        if moderngl is None:
            raise RuntimeError("moderngl not installed")
        self.ctx: moderngl.Context | None = None
        self.window = None
        self.prog = None
        self.line_prog = None
        self.atlas_size = 2048
        self.atlases: list[_Atlas] = []
        self.vbo = None
        self.instance_buffer = None
        self.vao = None
        self.draw_calls = 0
        self.shaders: dict[ShaderProgram, Any] = {}
        self.current_material: Material | None = None
        self.default_material: Material | None = None

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
        # 16 floats per instance -> 64 bytes
        self.instance_buffer = self.ctx.buffer(reserve=64 * 4096)

        vert_src = resources.files("sage_engine.render.shaders").joinpath("basic.vert").read_text()
        frag_src = resources.files("sage_engine.render.shaders").joinpath("basic.frag").read_text()
        shader = ShaderProgram("default", vert_src, frag_src)
        self.default_material = Material(shader)
        self.register_shader(shader)
        program = self.shaders[shader]
        self.prog = program
        self.vao = self.ctx.vertex_array(
            program,
            [
                (self.vbo, "2f", "in_vert"),
                (
                    self.instance_buffer,
                    "2f 2f 1f 1f 4f 1f 4f 1f/i",
                    "in_pos",
                    "in_scale",
                    "in_rot",
                    "in_atlas",
                    "in_uv",
                    "in_blend",
                    "in_color",
                    "in_depth",
                ),
            ],
        )
        self.current_material = self.default_material
        for i in range(8):
            name = f"u_tex[{i}]"
            if name in self.prog:
                self.prog[name].value = i
        self.prog["u_viewProj"].write(np.eye(3, dtype="f4").tobytes())

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
            for i, atlas in enumerate(self.atlases):
                atlas.texture.use(location=i)

    def draw_sprites(self, instances: NDArray) -> None:
        if self.ctx is None or self.vao is None:
            return
        arr = np.asarray(instances, dtype="f4")
        if arr.size == 0:
            return
        # split by blend mode so OpenGL blend function matches
        for blend in np.unique(arr[:, 10]).astype(int):
            subset = arr[arr[:, 10] == float(blend)]
            if subset.size == 0:
                continue
            self.instance_buffer.orphan(subset.nbytes)
            self.instance_buffer.write(subset.tobytes())
            if blend == 0:
                self.ctx.blend_func = moderngl.SRC_ALPHA, moderngl.ONE_MINUS_SRC_ALPHA
            else:
                self.ctx.blend_func = moderngl.ONE, moderngl.ONE_MINUS_SRC_ALPHA
            self.vao.render(moderngl.TRIANGLE_STRIP, instances=len(subset))
            self.draw_calls += 1

    # material API ------------------------------------------------------------
    def register_shader(self, program: ShaderProgram) -> None:
        if self.ctx is None:
            return
        if program in self.shaders:
            return
        self.shaders[program] = self.ctx.program(
            vertex_shader=program.vertex_source, fragment_shader=program.fragment_source
        )

    def set_material(self, material: Material) -> None:
        if self.ctx is None:
            return
        self.current_material = material
        if material.shader not in self.shaders:
            self.register_shader(material.shader)
        prog = self.shaders[material.shader]
        self.prog = prog
        self.vao = self.ctx.vertex_array(
            prog,
            [
                (self.vbo, "2f", "in_vert"),
                (
                    self.instance_buffer,
                    "2f 2f 1f 1f 4f 1f 4f 1f/i",
                    "in_pos",
                    "in_scale",
                    "in_rot",
                    "in_atlas",
                    "in_uv",
                    "in_blend",
                    "in_color",
                    "in_depth",
                ),
            ],
        )
        for i in range(8):
            name = f"u_tex[{i}]"
            if name in prog:
                prog[name].value = i
        for name, value in material.uniforms.items():
            if name in prog:
                prog[name].value = value if not isinstance(value, (list, tuple)) else tuple(value)
        if material.blend == "premultiplied":
            self.ctx.blend_func = moderngl.ONE, moderngl.ONE_MINUS_SRC_ALPHA
        else:
            self.ctx.blend_func = moderngl.SRC_ALPHA, moderngl.ONE_MINUS_SRC_ALPHA

    def draw_material_group(self, instances: NDArray) -> None:
        self.draw_sprites(instances)

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

    def _alloc(self, atlas: _Atlas, w: int, h: int) -> tuple[int, int] | None:
        if atlas.next_x + w > self.atlas_size:
            atlas.next_x = 0
            atlas.next_y += atlas.row_h
            atlas.row_h = 0
        if atlas.next_y + h > self.atlas_size:
            return None
        x, y = atlas.next_x, atlas.next_y
        atlas.next_x += w
        atlas.row_h = max(atlas.row_h, h)
        return x, y

    def create_texture(self, image: Any) -> tuple[int, tuple[float, float, float, float]]:
        if self.ctx is None:
            return 0, (0.0, 0.0, 1.0, 1.0)
        img = image.convert("RGBA")
        w, h = img.size
        for i, atlas in enumerate(self.atlases):
            pos = self._alloc(atlas, w, h)
            if pos is not None:
                x, y = pos
                atlas.texture.write(img.tobytes(), viewport=(x, y, w, h))
                u0 = x / self.atlas_size
                v0 = y / self.atlas_size
                u1 = (x + w) / self.atlas_size
                v1 = (y + h) / self.atlas_size
                return i, (u0, v0, u1, v1)
        tex = self.ctx.texture((self.atlas_size, self.atlas_size), 4)
        tex.filter = (moderngl.NEAREST, moderngl.NEAREST)
        tex.use(location=len(self.atlases))
        atlas = _Atlas(tex)
        self.atlases.append(atlas)
        pos = self._alloc(atlas, w, h)
        if pos is None:
            raise RuntimeError("atlas full")
        x, y = pos
        atlas.texture.write(img.tobytes(), viewport=(x, y, w, h))
        u0 = x / self.atlas_size
        v0 = y / self.atlas_size
        u1 = (x + w) / self.atlas_size
        v1 = (y + h) / self.atlas_size
        return len(self.atlases) - 1, (u0, v0, u1, v1)

    def set_camera(self, matrix: Sequence[float]) -> None:
        if self.prog is not None:
            self.prog["u_viewProj"].write(np.array(matrix, dtype="f4").tobytes())

__all__ = ["OpenGLBackend"]
