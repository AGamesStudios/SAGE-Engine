"""Simple physics integration using optional ``pymunk``."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any

from .core.extensions import EngineExtension

try:  # optional dependency
    import pymunk
except Exception:  # pragma: no cover - pymunk missing
    pymunk = None  # type: ignore


__all__ = ["PhysicsWorld", "PhysicsExtension"]


@dataclass
class PhysicsBody:
    obj: Any
    body: Any


class PhysicsWorld:
    """Wrapper around :mod:`pymunk` for simple 2D physics."""

    def __init__(self, gravity: tuple[float, float] = (0, -900)) -> None:
        if pymunk is None:
            raise ImportError("pymunk is required for physics")
        self.space = pymunk.Space()
        self.space.gravity = gravity
        self.bodies: list[PhysicsBody] = []

    def add_box(
        self,
        obj: Any,
        size: tuple[float, float] = (1, 1),
        mass: float = 1.0,
        body_type: str = "dynamic",
    ) -> PhysicsBody:
        if pymunk is None:
            raise ImportError("pymunk is required for physics")
        if body_type == "static":
            body = pymunk.Body(body_type=pymunk.Body.STATIC)
        else:
            body = pymunk.Body(mass, pymunk.moment_for_box(mass, size))
        body.position = (getattr(obj, "x", 0.0), getattr(obj, "y", 0.0))
        shape = pymunk.Poly.create_box(body, size)
        self.space.add(body, shape)
        pb = PhysicsBody(obj, body)
        self.bodies.append(pb)
        return pb

    def step(self, dt: float) -> None:
        if pymunk is None:
            raise ImportError("pymunk is required for physics")
        self.space.step(dt)
        for pb in self.bodies:
            obj = pb.obj
            pos = pb.body.position
            if hasattr(obj, "x"):
                obj.x = float(pos.x)
            if hasattr(obj, "y"):
                obj.y = float(pos.y)


class PhysicsExtension(EngineExtension):
    """Engine extension that steps a :class:`PhysicsWorld`."""

    def __init__(self, world: PhysicsWorld | None = None) -> None:
        self.world = world or PhysicsWorld()

    def update(self, engine, dt: float) -> None:  # type: ignore[override]
        self.world.step(dt)
