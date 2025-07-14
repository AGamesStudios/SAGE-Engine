"""Utility to pack multiple PNG images into a sprite atlas."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Iterable

from PIL import Image


def pack_atlas(images: Iterable[str], out_dir: str | Path = "build") -> Path:
    """Pack *images* into ``atlas.png`` and ``atlas.json`` in *out_dir*.

    Returns the path to ``atlas.png``.
    """
    out_path = Path(out_dir)
    out_path.mkdir(parents=True, exist_ok=True)
    loaded = []
    names: set[str] = set()
    for img in images:
        path = Path(img)
        if not path.is_file():
            raise FileNotFoundError(path)
        name = path.stem
        if name in names:
            raise ValueError(f"duplicate image name: {name}")
        names.add(name)
        loaded.append(Image.open(path).convert("RGBA"))
    if not loaded:
        raise ValueError("no images to pack")
    atlas_w = sum(im.width for im in loaded)
    atlas_h = max(im.height for im in loaded)
    atlas = Image.new("RGBA", (atlas_w, atlas_h))
    meta: dict[str, dict[str, int]] = {}
    x = 0
    for img_path, im in zip(images, loaded):
        atlas.paste(im, (x, 0))
        meta[Path(img_path).stem] = {"x": x, "y": 0, "w": im.width, "h": im.height}
        x += im.width
    atlas_file = out_path / "atlas.png"
    atlas.save(atlas_file)
    with open(out_path / "atlas.json", "w", encoding="utf-8") as f:
        json.dump(meta, f)
    return atlas_file


if __name__ == "__main__":  # pragma: no cover - CLI utility
    import argparse

    p = argparse.ArgumentParser(description="Pack images into an atlas")
    p.add_argument("output", help="output directory")
    p.add_argument("images", nargs="+", help="input PNG images")
    args = p.parse_args()
    try:
        path = pack_atlas(args.images, args.output)
    except Exception as exc:
        import sys

        print(exc, file=sys.stderr)
        raise SystemExit(1)
    print(f"Atlas written to {path}")
