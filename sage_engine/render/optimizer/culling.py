from __future__ import annotations

"""Basic bounding-box visibility checks."""

from typing import Any, Iterable, List, Tuple


BBox = Tuple[int, int, int, int]


def is_visible(bbox: BBox, viewport: BBox) -> bool:
    x0, y0, x1, y1 = bbox
    vx0, vy0, vx1, vy1 = viewport
    return not (x1 < vx0 or x0 > vx1 or y1 < vy0 or y0 > vy1)


def cull(objects: Iterable[Tuple[Any, BBox]], viewport: BBox) -> List[Any]:
    return [obj for obj, box in objects if is_visible(box, viewport)]
