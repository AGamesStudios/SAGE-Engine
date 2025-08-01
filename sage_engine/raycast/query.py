from __future__ import annotations

from math import hypot
from typing import Callable, Iterable, Iterator

from ..objects import runtime
from ..objects.object import Object

from .shape import Line, Circle, Rect
from .results import RaycastHit
from .core import line_rect_intersection, circle_rect_intersection

FilterFn = Callable[[Object], bool]


def _bounding_rect(obj: Object) -> Rect:
    if (role := obj.get_role("RectObject")) is not None:
        return Rect(obj.position.x, obj.position.y, role.width, role.height)
    w = float(obj.data.get("width", 1))
    h = float(obj.data.get("height", 1))
    return Rect(obj.position.x, obj.position.y, w, h)


def _iter_objects(filter_fn: FilterFn | None) -> Iterator[Object]:
    objs = runtime.store.objects.values()
    if filter_fn:
        return (o for o in objs if filter_fn(o))
    return iter(objs)


def cast_line(x1: float, y1: float, x2: float, y2: float,
              filter_fn: FilterFn | None = None) -> RaycastHit | None:
    if x1 == x2 and y1 == y2:
        return None
    line = Line(x1, y1, x2, y2)
    best: RaycastHit | None = None
    for obj in _iter_objects(filter_fn):
        rect = _bounding_rect(obj)
        res = line_rect_intersection(line, rect)
        if res:
            point, dist = res
            if best is None or dist < best.distance:
                best = RaycastHit(
                    object_id=obj.id,
                    point=point,
                    distance=dist,
                    tags=list(obj.data.get("tags", [])),
                    role=next(iter(obj.roles), None),
                )
    return best


def cast_circle(x: float, y: float, radius: float,
                filter_fn: FilterFn | None = None) -> list[RaycastHit]:
    circle = Circle(x, y, radius)
    hits: list[RaycastHit] = []
    for obj in _iter_objects(filter_fn):
        rect = _bounding_rect(obj)
        if circle_rect_intersection(circle, rect):
            dx = obj.position.x - x
            dy = obj.position.y - y
            dist = hypot(dx, dy)
            hits.append(
                RaycastHit(
                    object_id=obj.id,
                    point=(obj.position.x, obj.position.y),
                    distance=dist,
                    tags=list(obj.data.get("tags", [])),
                    role=next(iter(obj.roles), None),
                )
            )
    hits.sort(key=lambda h: h.distance)
    return hits
