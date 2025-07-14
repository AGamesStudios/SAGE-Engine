import hashlib
from pathlib import Path
import pytest

from tools import pack_atlas


def test_pack_atlas(tmp_path):
    import importlib
    import sys
    sys.modules.pop("PIL.Image", None)
    sys.modules.pop("PIL", None)
    importlib.invalidate_caches()
    pil_image = importlib.import_module("PIL.Image")
    importlib.reload(pack_atlas)
    if not hasattr(pil_image, "open"):
        import pytest

        pytest.skip("Pillow missing")
    imgs = []
    for i in range(2):
        path = tmp_path / f"img{i}.png"
        pil_image.new("RGBA", (2, 2), (i * 50, 0, 0, 255)).save(path)
        imgs.append(str(path))
    atlas = pack_atlas.pack_atlas(imgs, tmp_path)
    assert atlas.exists()
    data = atlas.read_bytes()
    md5 = hashlib.md5(data).hexdigest()
    assert len(data) > 10
    meta = Path(tmp_path / "atlas.json").read_text()
    assert "img0" in meta and "img1" in meta
    assert len(md5) == 32


def test_pack_atlas_missing(tmp_path):
    from PIL import Image

    img = tmp_path / "foo.png"
    Image.new("RGBA", (2, 2)).save(img)
    with pytest.raises(FileNotFoundError):
        pack_atlas.pack_atlas([img, tmp_path / "missing.png"], tmp_path)


def test_pack_atlas_duplicate(tmp_path):
    from PIL import Image

    img = tmp_path / "foo.png"
    Image.new("RGBA", (2, 2)).save(img)
    with pytest.raises(ValueError):
        pack_atlas.pack_atlas([img, img], tmp_path)
