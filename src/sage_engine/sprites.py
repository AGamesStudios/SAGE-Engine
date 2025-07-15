from __future__ import annotations

from dataclasses import dataclass
from typing import List, Any, Dict
from .transform import Transform
from .render.material import Material
from .render import shader as _shader
from pathlib import Path

_LAYER_SCALE = 0.01

_base = Path(__file__).resolve().parent / "render" / "shaders"
_vert_path = _base / "basic.vert"
_frag_path = _base / "basic.frag"
DEFAULT_MATERIAL = Material(_shader.load("default", _vert_path, _frag_path))

try:  # pragma: no cover - optional dependency
    import numpy as np  # type: ignore
    from numpy.typing import NDArray
except Exception:  # pragma: no cover - numpy optional
    np = None  # type: ignore
    NDArray = Any  # type: ignore


@dataclass
class Sprite:
    x: float
    y: float
    sx: float = 1.0
    sy: float = 1.0
    rot: float = 0.0
    atlas: float = 0.0
    uv: tuple[float, float, float, float] = (0.0, 0.0, 1.0, 1.0)
    blend: str = "alpha"
    color: tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0)
    layer: int = 0
    z: float = 0.0
    material: Material | None = None

    def set_transform(
        self,
        *,
        pos: tuple[float, float] | None = None,
        scale: tuple[float, float] | None = None,
        rot: float | None = None,
    ) -> None:
        if pos is not None:
            self.x, self.y = pos
        if scale is not None:
            self.sx, self.sy = scale
        if rot is not None:
            self.rot = rot

    def transform(self) -> Transform:
        return Transform((self.x, self.y), (self.sx, self.sy), self.rot)


_sprites: List[Sprite] = []


def add(sprite: Sprite) -> None:
    _sprites.append(sprite)


def clear() -> None:
    _sprites.clear()


def collect_instances() -> NDArray | list[list[float]]:
    ordered = sorted(_sprites, key=lambda s: (s.layer, s.z))
    if np is None:
        out = []
        for s in ordered:
            blend = 0.0 if s.blend == "alpha" else 1.0
            depth = s.layer * _LAYER_SCALE + s.z
            out.append([
                s.x,
                s.y,
                s.sx,
                s.sy,
                s.rot,
                s.atlas,
                *s.uv,
                blend,
                *s.color,
                depth,
            ])
        return out
    arr = np.zeros((len(ordered), 16), dtype=np.float32)
    for i, s in enumerate(ordered):
        blend = 0.0 if s.blend == "alpha" else 1.0
        depth = s.layer * _LAYER_SCALE + s.z
        arr[i] = (
            s.x,
            s.y,
            s.sx,
            s.sy,
            s.rot,
            s.atlas,
            *s.uv,
            blend,
            *s.color,
            depth,
        )
    return arr


def collect_groups() -> list[tuple[Material, NDArray | list[list[float]]]]:
    grouped: Dict[Material, List[list[float]]] = {}
    ordered = sorted(_sprites, key=lambda s: (s.layer, s.z))
    for s in ordered:
        blend = 0.0 if s.blend == "alpha" else 1.0
        depth = s.layer * _LAYER_SCALE + s.z
        row = [
            s.x,
            s.y,
            s.sx,
            s.sy,
            s.rot,
            s.atlas,
            *s.uv,
            blend,
            *s.color,
            depth,
        ]
        mat = s.material or DEFAULT_MATERIAL
        grouped.setdefault(mat, []).append(row)

    result: list[tuple[Material, NDArray | list[list[float]]]] = []
    for mat, rows in grouped.items():
        if np is None:
            result.append((mat, rows))
        else:
            result.append((mat, np.asarray(rows, dtype=np.float32)))
    return result
