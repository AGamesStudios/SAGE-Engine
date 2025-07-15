from PIL import Image

from sage_engine import resources, render


def test_get_texture_cached(tmp_path, monkeypatch):
    img_path = tmp_path / "img.png"
    Image.new("RGBA", (4, 4), (255, 0, 0, 255)).save(img_path)
    backend = render.get_backend("headless")
    backend.create_device(16, 16)
    calls = []

    def fake_create(image):
        calls.append(True)
        return len(calls) - 1

    monkeypatch.setattr(resources.manager, "backend", backend, raising=False)
    monkeypatch.setattr(backend, "create_texture", fake_create)
    resources.manager._cache.clear()

    t1 = resources.manager.get_texture(str(img_path))
    t2 = resources.manager.get_texture(str(img_path))
    assert t1 is t2
    assert len(calls) == 1
