from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Dict, List, Tuple

try:  # pragma: no cover - optional dependency
    import numpy as np
    from numpy.typing import NDArray
except Exception:  # pragma: no cover - numpy optional
    NDArray = Any  # type: ignore
    np = None  # type: ignore

from .render.font import Font
from .render.material import Material
from .sprites import DEFAULT_MATERIAL, _LAYER_SCALE


@dataclass
class TextObject:
    text: str
    x: float = 0.0
    y: float = 0.0
    font: Font | None = None
    color: Tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0)
    layer: int = 0
    z: float = 0.0
    material: Material | None = None


_texts: List[TextObject] = []


def add(text_obj: TextObject) -> None:
    _texts.append(text_obj)


def clear() -> None:
    _texts.clear()


def collect_groups() -> List[tuple[Material, NDArray | List[List[float]]]]:
    grouped: Dict[Material, List[List[float]]] = {}
    for obj in _texts:
        if obj.font is None:
            continue
        mat = obj.material or DEFAULT_MATERIAL
        x_cursor = obj.x
        for ch in obj.text:
            glyph = obj.font.glyphs.get(ord(ch))
            if glyph is None:
                x_cursor += obj.font.line_height
                continue
            depth = obj.layer * _LAYER_SCALE + obj.z
            row = [
                x_cursor + glyph.xoffset,
                obj.y - glyph.yoffset,
                float(glyph.width),
                float(glyph.height),
                0.0,
                float(obj.font.texture.atlas),
                *glyph.uv,
                0.0,
                *obj.color,
                depth,
            ]
            grouped.setdefault(mat, []).append(row)
            x_cursor += glyph.xadvance
    result: List[tuple[Material, NDArray | List[List[float]]]] = []
    for mat, rows in grouped.items():
        if np is None:
            result.append((mat, rows))
        else:
            result.append((mat, np.asarray(rows, dtype=np.float32)))
    return result


__all__ = ["TextObject", "add", "clear", "collect_groups"]
