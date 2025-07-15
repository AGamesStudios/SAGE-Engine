from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Tuple
import json

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
        self._atlas_slots: Dict[str, Texture] = {}
        self._atlas_textures: Dict[str, int] = {}
        self.backend = None

    def load_atlas(self, json_path: str) -> None:
        if self.backend is None:
            self.backend = render.get_backend()
        data = json.loads(Path(json_path).read_text())
        image_path = Path(json_path).with_suffix(".png")
        if "image" in data:
            image_path = Path(json_path).with_name(data["image"])
        img = Image.open(image_path)
        key = str(image_path)
        tex_id = self._atlas_textures.get(key)
        if tex_id is None:
            tex_id = self.backend.create_texture(img)
            self._atlas_textures[key] = tex_id
        w, h = data.get("size", img.size)
        for name, rect in data["sprites"].items():
            x, y, rw, rh = rect
            u0 = x / w
            v0 = y / h
            u1 = (x + rw) / w
            v1 = (y + rh) / h
            self._atlas_slots[name] = Texture(id=tex_id, uv=(u0, v0, u1, v1))

    def get_texture(self, path: str) -> Texture:
        if self.backend is None:
            self.backend = render.get_backend()
        atl = self._atlas_slots.get(path)
        if atl is not None:
            return atl
        key = str(Path(path))
        tex = self._cache.get(key)
        if tex is not None:
            return tex
        img = Image.open(key)
        tex_id = self.backend.create_texture(img)
        tex = Texture(id=tex_id, uv=(0.0, 0.0, 1.0, 1.0))
        self._cache[key] = tex
        return tex


manager = ResourceManager()


__all__ = ["Texture", "ResourceManager", "manager"]
