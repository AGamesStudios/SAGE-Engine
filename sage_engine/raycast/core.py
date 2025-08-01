from __future__ import annotations

from math import hypot
from typing import Tuple

from .shape import Line, Rect, Circle


def line_rect_intersection(line: Line, rect: Rect) -> Tuple[Tuple[float, float], float] | None:
    x1, y1, x2, y2 = line.x1, line.y1, line.x2, line.y2
    rx, ry, rw, rh = rect.x, rect.y, rect.width, rect.height

    dx = x2 - x1
    dy = y2 - y1
    tmin = 0.0
    tmax = 1.0

    for p, q in [(-dx, x1 - rx), (dx, rx + rw - x1), (-dy, y1 - ry), (dy, ry + rh - y1)]:
        if p == 0.0:
            if q < 0.0:
                return None
        else:
            r = q / p
            if p < 0:
                if r > tmax:
                    return None
                if r > tmin:
                    tmin = r
            else:
                if r < tmin:
                    return None
                if r < tmax:
                    tmax = r
    if tmin > tmax:
        return None
    ix = x1 + tmin * dx
    iy = y1 + tmin * dy
    return (ix, iy), hypot(ix - x1, iy - y1)


def circle_rect_intersection(circle: Circle, rect: Rect) -> bool:
    cx, cy, r = circle.x, circle.y, circle.radius
    rx, ry, rw, rh = rect.x, rect.y, rect.width, rect.height
    nearest_x = max(rx, min(cx, rx + rw))
    nearest_y = max(ry, min(cy, ry + rh))
    dx = cx - nearest_x
    dy = cy - nearest_y
    return dx * dx + dy * dy <= r * r
