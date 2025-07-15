from PIL import Image
import sage_engine.render as render
from sage_engine import resources

def test_multi_atlas(tmp_path, monkeypatch):
    backend = render.get_backend("headless")
    backend.create_device(16, 16)
    monkeypatch.setattr(resources.manager, "backend", backend, raising=False)
    calls = []
    def fake_create(img):
        idx = len(calls) // 2
        calls.append(1)
        return idx, (0.0, 0.0, 1.0, 1.0)
    monkeypatch.setattr(backend, "create_texture", fake_create)
    resources.manager._cache.clear()
    textures = []
    for i in range(5):
        img = Image.new("RGBA", (4, 4), (i, i, i, 255))
        path = tmp_path / f"img{i}.png"
        img.save(path)
        textures.append(resources.manager.get_texture(str(path)))
    assert len(set(t.atlas for t in textures)) > 1
