"""Camera and coordinates demo."""
from PIL import Image, ImageDraw

from sage_engine import render, sprites, camera, resources, ui


def main() -> None:
    img = Image.new("RGBA", (32, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.ellipse([0, 0, 31, 31], fill=(255, 0, 0, 255))
    path = "cam_sprite.png"
    img.save(path)

    backend = render.get_backend("headless")
    backend.create_device(200, 150)

    tex = resources.manager.get_texture(path)
    sprites.add(sprites.Sprite(x=5.0, y=0.0, atlas=float(tex.atlas), uv=tex.uv))
    label = ui.Label("UI stays put")
    label.x, label.y = 10.0, 10.0

    camera.set(0.0, 0.0, 1.0, aspect_ratio=200 / 150)
    camera.pan(2.0, 0.0)
    render.draw_frame(backend)


if __name__ == "__main__":
    main()
