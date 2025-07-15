"""Minimal Hello Sprite example."""
from PIL import Image, ImageDraw
import argparse

from sage_engine import render, sprites, camera, resources


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.parse_args()

    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rectangle([0, 0, 63, 63], fill=(255, 0, 0, 255))
    img_path = "hello_sprite.png"
    img.save(img_path)
    backend = render.get_backend("auto")
    backend.create_device(200, 200)
    tex = resources.manager.get_texture(img_path)
    sprites.add(sprites.Sprite(0.0, 0.0, tex_id=float(tex.id)))
    camera.set(0.0, 0.0, 1.0)
    render.draw_frame(backend)

if __name__ == "__main__":
    main()
