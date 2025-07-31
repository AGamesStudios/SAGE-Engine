from __future__ import annotations

"""Basic bounding-box visibility checks."""

from typing import Any, Iterable, List, Tuple
from ctypes import c_int, c_size_t

from .. import rustbridge as rust


BBox = Tuple[int, int, int, int]


def is_visible(bbox: BBox, viewport: BBox) -> bool:
    """Check visibility of ``bbox`` against ``viewport`` using Rust."""
    x0, y0, x1, y1 = bbox
    vx0, vy0, vx1, vy1 = viewport
    return bool(rust.lib.sage_is_visible(x0, y0, x1, y1, vx0, vy0, vx1, vy1))


def cull(objects: Iterable[Tuple[Any, BBox]], viewport: BBox) -> List[Any]:
    """Return only objects with bounding boxes visible in ``viewport`` via Rust."""
    objs = list(objects)
    boxes = (c_int * (len(objs) * 4))()
    for idx, (_, box) in enumerate(objs):
        boxes[idx * 4 + 0] = box[0]
        boxes[idx * 4 + 1] = box[1]
        boxes[idx * 4 + 2] = box[2]
        boxes[idx * 4 + 3] = box[3]
    vp = (c_int * 4)(*viewport)
    out = (c_size_t * len(objs))()
    count = rust.lib.sage_cull(boxes, len(objs), vp, out)
    visible = [objs[out[i]][0] for i in range(count)]
    return visible
