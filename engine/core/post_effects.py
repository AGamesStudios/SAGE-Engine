# SPDX-License-Identifier: MIT
"""Registry for post-processing effects."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Dict
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

class PerspectivePanoramaEffect(PostEffect):
    """Blend an equirectangular panorama with the scene."""

    def __init__(self):
        self._program = None

    def apply(self, renderer, texture: int, width: int, height: int,
              camera, params: dict) -> int:
        from OpenGL.GL import (
            glUseProgram, glBindVertexArray, glDrawArrays,
            glActiveTexture, glBindTexture, glGetUniformLocation,
            glUniform1i, glUniform1f, glUniform2f,
            GL_TEXTURE0, GL_TEXTURE1,
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
                }"""
            frag = """
                #version 120
                varying vec2 v_pos;
                uniform sampler2D colorTex;
                uniform sampler2D panoTex;
                uniform vec2 cam;
                uniform vec2 size;
                uniform vec2 factor;
                uniform float blend;
                uniform float y_sign;
                const float PI = 3.14159265;
                void main(){
                    vec2 uv = (v_pos + 1.0) * 0.5;
                    vec4 color = texture2D(colorTex, uv);
                    float wx = cam.x - size.x/2.0 + uv.x * size.x;
                    float wy = cam.y - size.y/2.0 + uv.y * size.y;
                    wy *= y_sign;
                    float lon = wx * factor.x;
                    float lat = wy * factor.y;
                    float u = mod(lon / (2.0 * PI), 1.0);
                    float v = 0.5 - lat / PI;
                    vec4 pano = texture2D(panoTex, vec2(u,v));
                    gl_FragColor = mix(color, pano, blend);
                }"""
            self._program = compileProgram(
                compileShader(vert, GL_VERTEX_SHADER),
                compileShader(frag, GL_FRAGMENT_SHADER),
            )
        glUseProgram(self._program)
        loc_color = glGetUniformLocation(self._program, "colorTex")
        loc_pano = glGetUniformLocation(self._program, "panoTex")
        loc_cam = glGetUniformLocation(self._program, "cam")
        loc_size = glGetUniformLocation(self._program, "size")
        loc_fac = glGetUniformLocation(self._program, "factor")
        loc_blend = glGetUniformLocation(self._program, "blend")
        loc_sign = glGetUniformLocation(self._program, "y_sign")
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D, texture)
        glUniform1i(loc_color, 0)
        pano_path = params.get("texture")
        tex = renderer._get_panorama_texture(pano_path) if pano_path else 0
        glActiveTexture(GL_TEXTURE1)
        glBindTexture(GL_TEXTURE_2D, tex)
        glUniform1i(loc_pano, 1)
        glUniform2f(loc_cam, camera.x, camera.y)
        glUniform2f(loc_size, float(camera.width), float(camera.height))
        glUniform2f(loc_fac, params.get("factor_x", 0.01), params.get("factor_y", 0.01))
        glUniform1f(loc_blend, params.get("blend", 1.0))
        glUniform1f(loc_sign, 1.0 if renderer.units_y_up() else -1.0)
        glBindVertexArray(renderer._pano_vao)
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4)
        glBindVertexArray(0)
        glUseProgram(0)
        return texture

register_post_effect("perspective_panorama", PerspectivePanoramaEffect())

__all__ = [
    "PostEffect",
    "register_post_effect",
    "get_post_effect",
    "POST_EFFECT_REGISTRY",
]

