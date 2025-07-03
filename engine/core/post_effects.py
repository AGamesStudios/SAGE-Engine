# SPDX-License-Identifier: MIT
"""Registry for post-processing effects."""

from dataclasses import dataclass
from typing import Dict
import ctypes
import logging

logger = logging.getLogger(__name__)

POST_EFFECT_REGISTRY: Dict[str, "PostEffect"] = {}


def register_post_effect(name: str, effect: "PostEffect") -> None:
    """Register a post-processing effect under ``name``."""
    POST_EFFECT_REGISTRY[name] = effect


def get_post_effect(name: str) -> "PostEffect | None":
    """Return the effect associated with ``name``."""
    return POST_EFFECT_REGISTRY.get(name)


@dataclass
class PostEffect:
    """Base class for post-processing effects."""

    def apply(self, renderer, texture: int, width: int, height: int,
              camera, params: dict) -> int:
        """Draw ``texture`` using this effect and return the result texture."""
        return texture


class GrayscaleEffect(PostEffect):
    """Convert the rendered frame to grayscale."""

    def __init__(self):
        self._program = None

    def apply(self, renderer, texture: int, width: int, height: int,
              camera, params: dict) -> int:
        from OpenGL.GL import (
            glUseProgram, glDrawArrays,
            glActiveTexture, glBindTexture, glGetUniformLocation,
            glUniform1i, GL_TEXTURE0, GL_TEXTURE_2D,
            glBindBuffer, glEnableVertexAttribArray, glVertexAttribPointer,
            glGetAttribLocation, GL_ARRAY_BUFFER, GL_FLOAT, GL_TRIANGLE_FAN,
        )
        if self._program is None:
            from OpenGL.GL import GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
            from OpenGL.GL.shaders import compileProgram, compileShader
            vert = """
                #version 120
                attribute vec2 pos;
                varying vec2 v_pos;
                void main(){
                    gl_Position = vec4(pos,0.0,1.0);
                    v_pos = pos;
                }
            """
            frag = """
                #version 120
                varying vec2 v_pos;
                uniform sampler2D colorTex;
                void main(){
                    vec2 uv = (v_pos + 1.0) * 0.5;
                    vec4 c = texture2D(colorTex, uv);
                    float g = dot(c.rgb, vec3(0.299, 0.587, 0.114));
                    gl_FragColor = vec4(g, g, g, c.a);
                }
            """
            self._program = compileProgram(
                compileShader(vert, GL_VERTEX_SHADER),
                compileShader(frag, GL_FRAGMENT_SHADER),
            )
        glUseProgram(self._program)
        loc_color = glGetUniformLocation(self._program, "colorTex")
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, texture)
        glUniform1i(loc_color, 0)
        glBindBuffer(GL_ARRAY_BUFFER, renderer._quad_vbo)
        loc = glGetAttribLocation(self._program, "pos")
        glEnableVertexAttribArray(loc)
        glVertexAttribPointer(loc, 2, GL_FLOAT, False, 0, ctypes.c_void_p(0))
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4)
        glBindBuffer(GL_ARRAY_BUFFER, 0)
        glUseProgram(0)
        return texture


register_post_effect("grayscale", GrayscaleEffect())


__all__ = [
    "PostEffect",
    "register_post_effect",
    "get_post_effect",
    "POST_EFFECT_REGISTRY",
]

