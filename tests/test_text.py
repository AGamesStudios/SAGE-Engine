from pathlib import Path
import json
from PIL import Image

from sage_engine.render.font import load
from sage_engine import text, render


def make_font(tmp: Path) -> Path:
    img = Image.new("RGBA", (8, 8), (255, 255, 255, 255))
    img_path = tmp / "font.png"
    img.save(img_path)
    data = {
        "pages": ["font.png"],
        "common": {"lineHeight": 8, "scaleW": 8, "scaleH": 8},
        "chars": [
            {"id": 65, "x": 0, "y": 0, "width": 8, "height": 8, "xoffset": 0, "yoffset": 0, "xadvance": 8, "page": 0}
        ],
    }
    json_path = tmp / "font.json"
    json_path.write_text(json.dumps(data))
    return json_path


def test_font_load(tmp_path, monkeypatch):
    json_path = make_font(tmp_path)
    backend = render.get_backend("headless")
    backend.create_device(16, 16)
    monkeypatch.setattr(text, "_texts", [], raising=False)
    font = load(json_path)
    assert 65 in font.glyphs
    glyph = font.glyphs[65]
    assert glyph.uv == (0.0, 0.0, 1.0, 1.0)


def test_text_object_instances(tmp_path, monkeypatch):
    json_path = make_font(tmp_path)
    backend = render.get_backend("headless")
    backend.create_device(16, 16)
    font = load(json_path)
    obj = text.TextObject("AA", 0.0, 0.0, font)
    monkeypatch.setattr(text, "_texts", [obj], raising=False)
    groups = text.collect_groups()
    assert len(groups) == 1
    mat, inst = groups[0]
    assert len(inst) == 2
    row = inst[0] if not hasattr(inst, "shape") else inst[0]
    assert row[10] == 2.0
