"""Simple UI rendering example."""
from PIL import Image, ImageDraw

from sage_engine import render, sprites, resources, ui
from sage_engine.ui import theme


def main() -> None:
    img = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.ellipse([0, 0, 31, 31], fill=(0, 255, 0, 255))
    path = "ui_sprite.png"
    img.save(path)

    backend = render.get_backend("headless")
    backend.create_device(200, 150)

    tex = resources.manager.get_texture(path)
    sprites.add(sprites.Sprite(x=0.0, y=0.0, atlas=float(tex.atlas), uv=tex.uv))
    btn = ui.Button("Click me")
    btn.x, btn.y = 10.0, 10.0

    theme.set_theme(str(theme._current_path))
    render.draw_frame(backend)


if __name__ == "__main__":
    main()
