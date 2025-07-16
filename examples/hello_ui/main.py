"""Simple UI rendering example."""
from pathlib import Path
import json
from PIL import Image, ImageDraw, ImageFont

from sage_engine import render, sprites, resources, ui
from sage_engine.ui import theme


def main() -> None:
    img = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.ellipse([0, 0, 31, 31], fill=(0, 255, 0, 255))
    icon_path = "ui_sprite.png"
    img.save(icon_path)

    tmp = Path("build")
    tmp.mkdir(exist_ok=True)
    font = ImageFont.load_default()
    w, h = font.getsize("W")
    atlas = Image.new("RGBA", (w, h))
    ImageDraw.Draw(atlas).text((0, 0), "A", font=font, fill=(255, 255, 255, 255))
    atlas_path = tmp / "font.png"
    atlas.save(atlas_path)
    data = {
        "pages": ["font.png"],
        "common": {"lineHeight": h, "scaleW": atlas.width, "scaleH": atlas.height},
        "chars": [{"id": 65, "x": 0, "y": 0, "width": w, "height": h, "xoffset": 0, "yoffset": 0, "xadvance": w, "page": 0}],
    }
    json_path = tmp / "font.json"
    json_path.write_text(json.dumps(data))
    theme.set_theme(str(theme._current_path))
    theme.current.font["file"] = str(json_path)

    backend = render.get_backend("headless")
    backend.create_device(200, 150)

    tex = resources.manager.get_texture(icon_path)
    sprites.add(sprites.Sprite(x=0.0, y=0.0, atlas=float(tex.atlas), uv=tex.uv))
    btn = ui.Button("A")
    btn.set_icon(icon_path)
    btn.x, btn.y = 10.0, 10.0

    theme.set_theme(str(theme._current_path))
    render.draw_frame(backend)


if __name__ == "__main__":
    main()
