from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Tuple

from PIL import Image

from . import render


@dataclass
class Texture:
    """Entry in the global texture atlas."""

    id: int
    uv: Tuple[float, float, float, float]


class ResourceManager:
    def __init__(self) -> None:
        self._cache: Dict[str, Texture] = {}
        self.backend = render.get_backend()

    def get_texture(self, path: str) -> Texture:
        key = str(Path(path))
        tex = self._cache.get(key)
        if tex is not None:
            return tex
        img = Image.open(key)
        tex_id = self.backend.create_texture(img)
        uv = (0.0, 0.0, 1.0, 1.0)
        if hasattr(self.backend, "uvs"):
            try:
                uv = self.backend.uvs[tex_id]
            except Exception:
                pass
        tex = Texture(id=tex_id, uv=uv)
        self._cache[key] = tex
        return tex


manager = ResourceManager()


__all__ = ["Texture", "ResourceManager", "manager"]
