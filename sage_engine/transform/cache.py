"""Very small helper providing temporary matrix buffers."""
from __future__ import annotations

from typing import List

from .math2d import identity

_pool: list[List[float]] = []


def get_temp_matrix() -> List[float]:
    try:
        return _pool.pop()
    except IndexError:
        return identity()


def release_temp_matrix(m: List[float]) -> None:
    _pool.append(m)
