from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Tuple
import json
import re

from ..resources import manager, Texture


@dataclass
class Glyph:
    uv: Tuple[float, float, float, float]
    xoffset: int
    yoffset: int
    xadvance: int
    width: int
    height: int


@dataclass
class Font:
    texture: Texture
    glyphs: Dict[int, Glyph]
    line_height: int


def _parse_fnt(text: str, base_path: Path) -> tuple[str, Dict[int, Glyph], int]:
    image = ""
    scale_w = 1
    scale_h = 1
    line_height = 0
    glyphs: Dict[int, Glyph] = {}
    for line in text.splitlines():
        line = line.strip()
        if line.startswith("page"):
            m = re.search(r"file=\"([^\"]+)\"", line)
            if m:
                image = m.group(1)
        elif line.startswith("common"):
            vals = {k: v for k, v in (part.split("=") for part in line.split()[1:])}
            scale_w = int(vals.get("scaleW", 1))
            scale_h = int(vals.get("scaleH", 1))
            line_height = int(vals.get("lineHeight", 0))
        elif line.startswith("char "):
            vals = {k: v for k, v in (part.split("=") for part in line.split()[1:])}
            cid = int(vals["id"])
            x = int(vals["x"])
            y = int(vals["y"])
            w = int(vals["width"])
            h = int(vals["height"])
            uv = (x / scale_w, y / scale_h, (x + w) / scale_w, (y + h) / scale_h)
            glyphs[cid] = Glyph(
                uv=uv,
                xoffset=int(vals.get("xoffset", 0)),
                yoffset=int(vals.get("yoffset", 0)),
                xadvance=int(vals.get("xadvance", w)),
                width=w,
                height=h,
            )
    return image, glyphs, line_height


def _parse_json(data: dict, base_path: Path) -> tuple[str, Dict[int, Glyph], int]:
    pages = data.get("pages", [])
    image = pages[0] if pages else ""
    common = data.get("common", {})
    scale_w = common.get("scaleW", 1)
    scale_h = common.get("scaleH", 1)
    line_height = common.get("lineHeight", 0)
    glyphs: Dict[int, Glyph] = {}
    for ch in data.get("chars", []):
        cid = int(ch.get("id", 0))
        x = int(ch.get("x", 0))
        y = int(ch.get("y", 0))
        w = int(ch.get("width", 0))
        h = int(ch.get("height", 0))
        uv = (x / scale_w, y / scale_h, (x + w) / scale_w, (y + h) / scale_h)
        glyphs[cid] = Glyph(
            uv=uv,
            xoffset=int(ch.get("xoffset", 0)),
            yoffset=int(ch.get("yoffset", 0)),
            xadvance=int(ch.get("xadvance", w)),
            width=w,
            height=h,
        )
    return image, glyphs, line_height


def load(path: str | Path) -> Font:
    """Load a bitmap font from a .fnt or BMFont JSON file."""
    p = Path(path)
    data = p.read_text()
    if p.suffix == ".json":
        image, glyphs, line_height = _parse_json(json.loads(data), p.parent)
    else:
        image, glyphs, line_height = _parse_fnt(data, p.parent)
    img_path = p.parent / image
    texture = manager.get_texture(str(img_path))
    return Font(texture=texture, glyphs=glyphs, line_height=line_height)


__all__ = ["Glyph", "Font", "load"]
