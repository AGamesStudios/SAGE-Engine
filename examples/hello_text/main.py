"""Hello Text example generating a tiny bitmap font on the fly."""
from pathlib import Path
import json
from PIL import Image, ImageDraw, ImageFont

from sage_engine.render.font import load
from sage_engine import text, render


def make_font(tmp: Path) -> Path:
    font = ImageFont.load_default()
    chars = "Hello, world!"
    w, h = font.getsize("W")
    atlas = Image.new("RGBA", (w * len(chars), h))
    draw = ImageDraw.Draw(atlas)
    metrics = []
    x = 0
    for ch in chars:
        draw.text((x, 0), ch, font=font, fill=(255, 255, 255, 255))
        metrics.append({
            "id": ord(ch),
            "x": x,
            "y": 0,
            "width": w,
            "height": h,
            "xoffset": 0,
            "yoffset": 0,
            "xadvance": w,
            "page": 0,
        })
        x += w
    atlas_path = tmp / "font.png"
    atlas.save(atlas_path)
    data = {
        "pages": ["font.png"],
        "common": {"lineHeight": h, "scaleW": atlas.width, "scaleH": atlas.height},
        "chars": metrics,
    }
    json_path = tmp / "font.json"
    json_path.write_text(json.dumps(data))
    return json_path


def main() -> None:
    tmp = Path("./build")
    tmp.mkdir(exist_ok=True)
    font_path = make_font(tmp)
    font = load(font_path)
    obj = text.TextObject("Hello, world!", 0.0, 0.0, font)
    text.add(obj)

    backend = render.get_backend("headless")
    backend.create_device(200, 100)
    render.draw_frame(backend)


if __name__ == "__main__":
    main()
