from __future__ import annotations

from .texture import Texture

from ..render import stats as render_stats

class TextureCache:
    _cache: dict[str, Texture] = {}
    _atlas_cache: dict[str, "TextureAtlas"] = {}

    @classmethod
    def load(cls, path: str) -> Texture:
        tex = cls._cache.get(path)
        if tex is None:
            tex = Texture()
            tex.load(path)
            render_stats.stats["textures_loaded"] += 1
            cls._cache[path] = tex
        return tex

    @classmethod
    def load_atlas(cls, path: str) -> "TextureAtlas":
        atlas = cls._atlas_cache.get(path)
        if atlas is None:
            from .atlas import TextureAtlas

            atlas = TextureAtlas()
            atlas.load(path)
            render_stats.stats["textures_loaded"] += 1
            cls._atlas_cache[path] = atlas
        return atlas

    @classmethod
    def memory_usage(cls) -> int:
        usage = 0
        for tex in cls._cache.values():
            usage += len(tex.pixels or b"")
        for atlas in cls._atlas_cache.values():
            usage += len(atlas.texture.pixels or b"")
        return usage

    @classmethod
    def clear(cls) -> None:
        cls._cache.clear()
        cls._atlas_cache.clear()
        render_stats.stats["texture_memory_kb"] = 0
