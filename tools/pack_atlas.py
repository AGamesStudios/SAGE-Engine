#!/usr/bin/env python3
from __future__ import annotations
import argparse
import json
from pathlib import Path
from PIL import Image

ATLAS_SIZE = 2048

def pack(images: list[Path], atlas_png: Path, atlas_json: Path) -> None:
    atlas = Image.new("RGBA", (ATLAS_SIZE, ATLAS_SIZE), (0, 0, 0, 0))
    next_x = 0
    next_y = 0
    row_h = 0
    sprites: dict[str, list[int]] = {}
    for img_path in images:
        img = Image.open(img_path).convert("RGBA")
        w, h = img.size
        if next_x + w > ATLAS_SIZE:
            next_x = 0
            next_y += row_h
            row_h = 0
        if next_y + h > ATLAS_SIZE:
            raise RuntimeError("atlas full")
        atlas.paste(img, (next_x, next_y))
        sprites[img_path.stem] = [next_x, next_y, w, h]
        next_x += w
        row_h = max(row_h, h)
    atlas_png.parent.mkdir(parents=True, exist_ok=True)
    atlas.save(atlas_png)
    data = {"image": atlas_png.name, "size": [ATLAS_SIZE, ATLAS_SIZE], "sprites": sprites}
    atlas_json.write_text(json.dumps(data))


def main() -> None:
    parser = argparse.ArgumentParser(description="Pack images into an atlas")
    parser.add_argument("images", nargs="+")
    parser.add_argument("--out", default="build/atlas")
    args = parser.parse_args()
    out = Path(args.out)
    pack([Path(p) for p in args.images], out / "atlas.png", out / "atlas.json")


if __name__ == "__main__":
    main()
