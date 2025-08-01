from pathlib import Path

from sage_engine.texture import TextureCache
from sage_engine.format import sageimg


def test_texture_atlas_region(tmp_path: Path):
    img_path = tmp_path / "atlas.sageimg"
    img_path.write_bytes(sageimg.encode(b"\x00" * 16, 2, 2))
    (tmp_path / "atlas.sageimg.meta").write_text('{"icon": [0, 0, 1, 1]}')
    atlas = TextureCache.load_atlas(str(img_path))
    region = atlas.get_region("icon")
    assert region == (0, 0, 1, 1)
