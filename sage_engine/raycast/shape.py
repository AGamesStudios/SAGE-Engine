from dataclasses import dataclass
from typing import List, Tuple


@dataclass(slots=True)
class Line:
    x1: float
    y1: float
    x2: float
    y2: float


@dataclass(slots=True)
class Rect:
    x: float
    y: float
    width: float
    height: float


@dataclass(slots=True)
class Circle:
    x: float
    y: float
    radius: float


@dataclass(slots=True)
class Polygon:
    points: List[Tuple[float, float]]
