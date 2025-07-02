"""Shader utilities for the OpenGL renderer."""
from __future__ import annotations


import ctypes
from ..shader import Shader


VERT_SHADER = """
    #version 120
    attribute vec2 pos;
    attribute vec2 uv;
    varying vec2 v_uv;
    void main() {
        gl_Position = vec4(pos, 0.0, 1.0);
        v_uv = uv;
    }
"""

FRAG_SHADER = """
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


def create_sprite_shader() -> Shader:
    """Return the default sprite shader."""
    return Shader(VERT_SHADER, FRAG_SHADER)


def setup_sprite_vao(shader: Shader) -> tuple[int, int]:
    """Create a VAO/VBO for sprite rendering and return their IDs."""
    from OpenGL.GL import (
        glGenVertexArrays,
        glBindVertexArray,
        glGenBuffers,
        glBindBuffer,
        glBufferData,
        glGetAttribLocation,
        glEnableVertexAttribArray,
        glVertexAttribPointer,
        GL_ARRAY_BUFFER,
        GL_STATIC_DRAW,
        GL_FLOAT,
    )

    program = shader.compile()
    vao = glGenVertexArrays(1)
    glBindVertexArray(vao)
    vbo = glGenBuffers(1)
    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    data = (ctypes.c_float * 16)(
        -0.5, -0.5, 0.0, 0.0,
        0.5, -0.5, 1.0, 0.0,
        0.5, 0.5, 1.0, 1.0,
        -0.5, 0.5, 0.0, 1.0,
    )
    glBufferData(GL_ARRAY_BUFFER, ctypes.sizeof(data), data, GL_STATIC_DRAW)
    loc_pos = glGetAttribLocation(program, "pos")
    loc_uv = glGetAttribLocation(program, "uv")
    glEnableVertexAttribArray(loc_pos)
    glVertexAttribPointer(loc_pos, 2, GL_FLOAT, False, 16, ctypes.c_void_p(0))
    glEnableVertexAttribArray(loc_uv)
    glVertexAttribPointer(loc_uv, 2, GL_FLOAT, False, 16, ctypes.c_void_p(8))
    glBindBuffer(GL_ARRAY_BUFFER, 0)
    glBindVertexArray(0)
    return vao, vbo
