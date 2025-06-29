from __future__ import annotations

from dataclasses import dataclass

from OpenGL.GL import GL_VERTEX_SHADER, GL_FRAGMENT_SHADER
from OpenGL.GL.shaders import compileProgram, compileShader

from ..log import logger


@dataclass
class Shader:
    """Simple wrapper for an OpenGL shader program."""

    vertex_source: str
    fragment_source: str
    program: int | None = None

    def compile(self) -> int:
        """Compile the shader program if not already compiled."""
        if self.program is not None:
            return self.program
        logger.debug("Compiling shader program")
        self.program = compileProgram(
            compileShader(self.vertex_source, GL_VERTEX_SHADER),
            compileShader(self.fragment_source, GL_FRAGMENT_SHADER),
        )
        logger.info("Shader compiled: %s", self.program)
        return self.program

    @classmethod
    def from_files(cls, vert_path: str, frag_path: str) -> "Shader":
        """Load shader sources from files."""
        with open(vert_path, "r", encoding="utf-8") as vf:
            vert = vf.read()
        with open(frag_path, "r", encoding="utf-8") as ff:
            frag = ff.read()
        return cls(vert, frag)
