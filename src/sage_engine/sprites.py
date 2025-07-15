from __future__ import annotations

from dataclasses import dataclass
from typing import List, Any

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
    rot: float = 0.0
    tex_id: float = 0.0
    color: tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0)


_sprites: List[Sprite] = []


def add(sprite: Sprite) -> None:
    _sprites.append(sprite)


def clear() -> None:
    _sprites.clear()


def collect_instances() -> NDArray | list[list[float]]:
    if np is None:
        return [
            [s.x, s.y, s.rot, s.tex_id, *s.color]
            for s in _sprites
        ]
    arr = np.zeros((len(_sprites), 8), dtype=np.float32)
    for i, s in enumerate(_sprites):
        arr[i] = (s.x, s.y, s.rot, s.tex_id, *s.color)
    return arr
