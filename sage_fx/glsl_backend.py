"""Stub GLSL generator for Feather-FX."""
from __future__ import annotations

from typing import Iterable

from .parser import Operation


def generate_glsl(ops: Iterable[Operation]) -> str:
    """Return a simple GLSL fragment shader."""
    lines = [
        "#version 120",
        "uniform sampler2D u_texture;",
        "varying vec2 v_uv;",
        "void main() {",
        "    vec4 color = texture2D(u_texture, v_uv);",
    ]
    for op in ops:
        if op.name == "blend_add":
            lines.append(f"    color += {op.args['factor']};")
        elif op.name == "blend_mul":
            lines.append(f"    color *= {op.args['factor']};")
        # other ops ignored for now
    lines.append("    gl_FragColor = color;")
    lines.append("}")
    return "\n".join(lines)

__all__ = ["generate_glsl"]
