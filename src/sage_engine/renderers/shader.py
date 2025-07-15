
from dataclasses import dataclass
from typing import cast

from OpenGL.GL import (
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER,
    glUseProgram,
    glGetUniformLocation,
    glUniform1f,
    glUniform2f,
    glUniform3f,
    glUniform4f,
)  # type: ignore[import-not-found]
from OpenGL.GL.shaders import compileProgram, compileShader  # type: ignore[import-not-found]

from ..utils.log import logger


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
        self.program = cast(
            int,
            compileProgram(
                compileShader(self.vertex_source, GL_VERTEX_SHADER),
                compileShader(self.fragment_source, GL_FRAGMENT_SHADER),
            ),
        )
        logger.info("Shader compiled: %s", self.program)
        return self.program

    def use(self, uniforms: dict | None = None) -> None:
        """Activate the shader and optionally upload ``uniforms``."""
        prog = self.compile()
        glUseProgram(prog)
        if not uniforms:
            return
        for name, val in uniforms.items():
            loc = glGetUniformLocation(prog, name)
            if loc < 0:
                continue
            if isinstance(val, (list, tuple)):
                if len(val) == 2:
                    glUniform2f(loc, *val)
                elif len(val) == 3:
                    glUniform3f(loc, *val)
                elif len(val) == 4:
                    glUniform4f(loc, *val)
                else:
                    logger.warning("Unsupported uniform size for %s", name)
            else:
                glUniform1f(loc, float(val))

    @staticmethod
    def stop() -> None:
        """Deactivate the current shader program."""
        glUseProgram(0)

    @classmethod
    def from_files(cls, vert_path: str, frag_path: str) -> "Shader":
        """Load shader sources from files."""
        with open(vert_path, "r", encoding="utf-8") as vf:
            vert = vf.read()
        with open(frag_path, "r", encoding="utf-8") as ff:
            frag = ff.read()
        return cls(vert, frag)
