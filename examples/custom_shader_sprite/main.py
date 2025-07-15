"""Demo using a custom shader material."""
from pathlib import Path
from PIL import Image, ImageDraw

from sage_engine import render, sprites, camera, resources
from sage_engine.render.shader import load
from sage_engine.render.material import Material


def main() -> None:
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rectangle([0, 0, 63, 63], fill=(0, 0, 255, 255))
    img_path = Path(__file__).with_name("sprite.png")
    img.save(img_path)

    backend = render.get_backend("auto")
    backend.create_device(200, 200)
    tex = resources.manager.get_texture(str(img_path))
    vert = Path(__file__).with_name("custom.vert")
    frag = Path(__file__).with_name("custom.frag")
    shader = load("gray", vert, frag)
    material = Material(shader)
    backend.register_shader(shader)
    sprites.add(sprites.Sprite(0.0, 0.0, atlas=float(tex.atlas), uv=tex.uv, material=material))
    camera.set(0.0, 0.0, 1.0)
    render.draw_frame(backend)


if __name__ == "__main__":
    main()
