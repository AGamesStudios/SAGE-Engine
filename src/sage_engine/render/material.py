from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any, Dict

from .shader import ShaderProgram


@dataclass(eq=False)
class Material:
    """Rendering material using a shader and optional uniforms."""

    shader: ShaderProgram
    uniforms: Dict[str, Any] = field(default_factory=dict)
    blend: str = "alpha"


__all__ = ["Material"]
