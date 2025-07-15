from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Tuple


@dataclass
class ShaderProgram:
    """GLSL shader program source."""

    name: str
    vertex_source: str
    fragment_source: str


_CACHE: Dict[Tuple[str, str], "ShaderProgram"] = {}


def load(name: str, vert_path: str | Path, frag_path: str | Path) -> ShaderProgram:
    """Load and cache shader sources from disk."""
    key = (str(vert_path), str(frag_path))
    prog = _CACHE.get(key)
    if prog is None:
        vert_src = Path(vert_path).read_text()
        frag_src = Path(frag_path).read_text()
        prog = ShaderProgram(name, vert_src, frag_src)
        _CACHE[key] = prog
    return prog


__all__ = ["ShaderProgram", "load"]
