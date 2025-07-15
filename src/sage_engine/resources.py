from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Tuple
import json

from PIL import Image

from . import render


@dataclass
class Texture:
    """Location of an image inside a texture atlas."""

    atlas: int
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
        atlas_id = self._atlas_textures.get(key)
        if atlas_id is None:
            atlas_id, _ = self.backend.create_texture(img)
            self._atlas_textures[key] = atlas_id
        w, h = data.get("size", img.size)
        for name, rect in data["sprites"].items():
            x, y, rw, rh = rect
            u0 = x / w
            v0 = y / h
            u1 = (x + rw) / w
            v1 = (y + rh) / h
            self._atlas_slots[name] = Texture(atlas=atlas_id, uv=(u0, v0, u1, v1))

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
        atlas_id, uv = self.backend.create_texture(img)
        tex = Texture(atlas=atlas_id, uv=uv)
        self._cache[key] = tex
        return tex


manager = ResourceManager()


__all__ = ["Texture", "ResourceManager", "manager"]
