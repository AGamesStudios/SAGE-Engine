import json
from sage_engine.texture import TextureCache
from sage_engine.render import stats as render_stats
from sage_engine.format import sageimg


def test_texture_stat_updates(tmp_path):
    TextureCache.clear()
    start_loaded = render_stats.stats["textures_loaded"]
    pixels = b"\x00\x00\x00\xff" * 256  # 16x16
    img = tmp_path / "tex.sageimg"
    img.write_bytes(sageimg.encode(pixels, 16, 16))
    meta = tmp_path / "tex.sageimg.meta"
    meta.write_text(json.dumps({"region": [0, 0, 16, 16]}))
    tex = TextureCache.load(str(img))
    assert render_stats.stats["textures_loaded"] == start_loaded + 1
    assert render_stats.stats["memory_peak"] >= render_stats.stats["texture_memory_kb"]
    atlas = TextureCache.load_atlas(str(img))
    start_hits = render_stats.stats["atlas_hits"]
    start_miss = render_stats.stats["atlas_misses"]
    atlas.get_region("region")
    atlas.get_region("missing")
    assert render_stats.stats["atlas_hits"] == start_hits + 1
    assert render_stats.stats["atlas_misses"] == start_miss + 1
