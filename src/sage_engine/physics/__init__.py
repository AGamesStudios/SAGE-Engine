from __future__ import annotations

from dataclasses import dataclass
from typing import Callable, List


@dataclass
class Body:
    x: float = 0.0
    y: float = 0.0
    vy: float = 0.0
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
            if body.behaviour == "dynamic":
                body.vy -= self.gravity * dt
                body.y += body.vy * dt
                if body.y <= 0:
                    body.y = 0
                    body.vy = -body.vy
                    for other in self.bodies:
                        if other.behaviour == "sensor" and other.on_contact:
                            other.on_contact(body)

__all__ = ["World", "Body"]
