from PIL import Image
import sage_engine.render as render
from sage_engine import resources


def test_texture_deduplication(tmp_path, monkeypatch):
    backend = render.get_backend("headless")
    backend.create_device(16, 16)
    monkeypatch.setattr(resources.manager, "backend", backend, raising=False)
    calls = []

    def fake_create(img):
        calls.append(True)
        return 0, (0.0, 0.0, 1.0, 1.0)

    monkeypatch.setattr(backend, "create_texture", fake_create)
    resources.manager._cache.clear()
    resources.manager._hash_map.clear()

    img1 = Image.new("RGBA", (4, 4), (255, 0, 0, 255))
    path_a = tmp_path / "a.png"
    path_b = tmp_path / "b.png"
    img1.save(path_a)
    img1.save(path_b)

    tex_a = resources.manager.get_texture(str(path_a))
    tex_b = resources.manager.get_texture(str(path_b))
    assert tex_a is tex_b
    assert len(calls) == 1
