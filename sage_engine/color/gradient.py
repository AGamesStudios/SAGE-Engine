from __future__ import annotations

from .model import Color


class Gradient:
    """Linear color gradient."""

    def __init__(self, stops: list[tuple[float, Color]]):
        self.stops = sorted(stops, key=lambda s: s[0])

    def at(self, pos: float) -> Color:
        if not self.stops:
            return Color(0, 0, 0)
        if pos <= self.stops[0][0]:
            return self.stops[0][1]
        for idx, (p, color) in enumerate(self.stops):
            if pos <= p:
                p0, c0 = self.stops[idx - 1]
                t = (pos - p0) / (p - p0)
                r = int(c0.r + (color.r - c0.r) * t)
                g = int(c0.g + (color.g - c0.g) * t)
                b = int(c0.b + (color.b - c0.b) * t)
                a = int(c0.a + (color.a - c0.a) * t)
                return Color(r, g, b, a)
        return self.stops[-1][1]
