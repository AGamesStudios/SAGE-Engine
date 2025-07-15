import hashlib
import types
import sage_engine.render_fabric as rf

try:
    from PIL import Image, ImageDraw
except Exception as exc:  # pragma: no cover - optional dependency
    import pytest

    pytest.skip(f"Pillow missing: {exc}", allow_module_level=True)

EXPECTED_SHA = "ad397b294769282c79be03e612b9418bb1ad614ce5ed04c0c07e69ed1ff377df"

class PILRenderer:
    def __init__(self, width: int, height: int) -> None:
        self.img = Image.new("RGBA", (width, height), (0, 0, 0, 0))
        self.draw = ImageDraw.Draw(self.img)
        self.calls = 0

    def draw_sprites(self, sprites):
        self.calls += 1
        for _, (x, y, w, h) in sprites:
            self.draw.rectangle([x, y, x + w - 1, y + h - 1], fill=(255, 0, 0, 255))

def test_offscreen_checksum(tmp_path, monkeypatch):
    monkeypatch.setitem(__import__("sys").modules, "wgpu", None)
    renderer = PILRenderer(64, 64)
    monkeypatch.setitem(
        __import__("sys").modules,
        "engine.renderers.opengl_renderer",
        types.SimpleNamespace(OpenGLRenderer=lambda w, h: renderer),
    )
    fabric = rf.RenderFabric(width=64, height=64)
    assert fabric.backend == "opengl"
    for y in range(8):
        for x in range(8):
            fabric.sprite_pass.add_sprite(0, (x * 8.0, y * 8.0, 8.0, 8.0))
    fabric.sprite_pass.draw()
    out = tmp_path / "frame.png"
    renderer.img.save(out)
    sha = hashlib.sha256(out.read_bytes()).hexdigest()
    assert sha == EXPECTED_SHA
