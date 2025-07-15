from PIL import Image
import json

from sage_engine import resources, render


def test_get_texture_cached(tmp_path, monkeypatch):
    img_path = tmp_path / "img.png"
    Image.new("RGBA", (4, 4), (255, 0, 0, 255)).save(img_path)
    backend = render.get_backend("headless")
    backend.create_device(16, 16)
    calls = []

    def fake_create(image):
        calls.append(True)
        return 0, (0.0, 0.0, 1.0, 1.0)

    monkeypatch.setattr(resources.manager, "backend", backend, raising=False)
    monkeypatch.setattr(backend, "create_texture", fake_create)
    resources.manager._cache.clear()

    t1 = resources.manager.get_texture(str(img_path))
    t2 = resources.manager.get_texture(str(img_path))
    assert t1 is t2
    assert len(calls) == 1


def test_load_atlas(tmp_path, monkeypatch):
    img_a = Image.new("RGBA", (4, 4), (255, 0, 0, 255))
    img_b = Image.new("RGBA", (4, 4), (0, 255, 0, 255))
    img_a.save(tmp_path / "a.png")
    img_b.save(tmp_path / "b.png")
    atlas = Image.new("RGBA", (8, 4))
    atlas.paste(img_a, (0, 0))
    atlas.paste(img_b, (4, 0))
    atlas.save(tmp_path / "atlas.png")
    data = {
        "image": "atlas.png",
        "size": [8, 4],
        "sprites": {"a": [0, 0, 4, 4], "b": [4, 0, 4, 4]},
    }
    (tmp_path / "atlas.json").write_text(json.dumps(data))
    backend = render.get_backend("headless")
    backend.create_device(16, 16)
    monkeypatch.setattr(resources.manager, "backend", backend, raising=False)
    resources.manager._atlas_slots.clear()
    resources.manager._atlas_textures.clear()
    resources.manager.load_atlas(str(tmp_path / "atlas.json"))
    tex_a = resources.manager.get_texture("a")
    tex_b = resources.manager.get_texture("b")
    assert tex_a.atlas == tex_b.atlas == 0
    assert tex_a.uv != tex_b.uv
