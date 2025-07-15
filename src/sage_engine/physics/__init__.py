from __future__ import annotations

from dataclasses import dataclass
from typing import Callable, List


# Toggle for rendering physics in X-Ray mode
debug_xray: bool = False


def handle_key(key: str) -> None:
    """Toggle :data:`debug_xray` when *key* is ``'F3'``."""
    global debug_xray
    if key.upper() == "F3":
        debug_xray = not debug_xray


@dataclass
class Body:
    x: float = 0.0
    y: float = 0.0
    vy: float = 0.0
    prev_y: float = 0.0
    behaviour: str = "dynamic"  # dynamic, sensor, one_way
    on_contact: Callable[["Body"], None] | None = None


class World:
    def __init__(self, gravity: float = 9.8) -> None:
        self.gravity = gravity
        self.bodies: List[Body] = []

    def create_box(self, x: float = 0.0, y: float = 0.0, *, behaviour: str = "dynamic") -> Body:
        body = Body(x=x, y=y, behaviour=behaviour)
        self.bodies.append(body)
        return body

    def step(self, dt: float) -> None:
        for body in self.bodies:
            body.prev_y = body.y
        for body in self.bodies:
            if body.behaviour == "dynamic":
                body.vy -= self.gravity * dt
                body.y += body.vy * dt
                for other in self.bodies:
                    if other.behaviour == "one_way" and body.prev_y >= other.y > body.y and body.vy < 0:
                        body.y = other.y
                        body.vy = 0
                if body.y <= 0:
                    body.y = 0
                    body.vy = -body.vy
                    for other in self.bodies:
                        if other.behaviour == "sensor" and other.on_contact:
                            other.on_contact(body)


def collect_debug_lines(world: "World") -> list[float]:
    """Return simple line segments for each body."""
    verts: list[float] = []
    for body in world.bodies:
        x = body.x
        y = body.y
        s = 0.5
        verts += [x - s, y - s, x + s, y - s, x + s, y + s, x - s, y + s, x - s, y - s]
    return verts

__all__ = [
    "World",
    "Body",
    "debug_xray",
    "handle_key",
    "collect_debug_lines",
]
