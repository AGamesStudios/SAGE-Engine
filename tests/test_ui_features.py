import json
from pathlib import Path
from PIL import Image
from sage_engine import ui


def make_font(tmp: Path) -> Path:
    img = Image.new("RGBA", (8, 8), (255, 255, 255, 255))
    img_path = tmp / "f.png"
    img.save(img_path)
    data = {
        "pages": ["f.png"],
        "common": {"lineHeight": 8, "scaleW": 8, "scaleH": 8},
        "chars": [{"id": 65, "x": 0, "y": 0, "width": 8, "height": 8, "xoffset": 0, "yoffset": 0, "xadvance": 8, "page": 0}],
    }
    json_path = tmp / "f.json"
    json_path.write_text(json.dumps(data))
    return json_path


def test_hover_active_and_icon(tmp_path, monkeypatch):
    # prepare theme with hover/active and font
    font_path = make_font(tmp_path)
    theme_file = tmp_path / "t.vel"
    theme_file.write_text(
        f"""colors:\n  bg: '#000000'\n  fg: '#ffffff'\n  hover: '#111111'\n  active: '#222222'\nfont:\n  file: '{font_path}'\n  size: 8\n""")
    ui.theme.set_theme(str(theme_file))
    btn = ui.Button("A")
    assert btn.text_obj is not None  # font from theme
    monkeypatch.setattr(ui, "_widgets", [btn], raising=False)
    # create icon
    img = Image.new("RGBA", (4, 4), (255, 0, 0, 255))
    icon_path = tmp_path / "icon.png"
    img.save(icon_path)
    btn.set_icon(str(icon_path))
    assert btn.icon is not None
    btn.hover(True)
    inst = ui.collect_instances()
    rgba_hover = ui.theme.color_rgba("#111111")
    arr = inst[0] if not hasattr(inst, "shape") else inst[0]
    assert all(abs(arr[11 + i] - rgba_hover[i]) < 1e-6 for i in range(4))
    btn.click()
    inst = ui.collect_instances()
    rgba_active = ui.theme.color_rgba("#222222")
    arr = inst[0] if not hasattr(inst, "shape") else inst[0]
    assert all(abs(arr[11 + i] - rgba_active[i]) < 1e-6 for i in range(4))
    # icon reference stored

