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
    shape: Any
    base_size: tuple[float, float]
    size: tuple[float, float]


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
        pb = PhysicsBody(obj=obj, body=body, shape=shape, base_size=size, size=size)
        self.bodies.append(pb)
        return pb

    def remove(self, pb: PhysicsBody) -> None:
        """Remove *pb* from the world and internal list."""
        if pymunk is None:
            raise ImportError("pymunk is required for physics")
        if pb in self.bodies:
            shapes = list(pb.body.shapes)
            self.space.remove(pb.body, *shapes)
            self.bodies.remove(pb)

    def step(self, dt: float) -> None:
        if pymunk is None:
            raise ImportError("pymunk is required for physics")
        import math
        for pb in self.bodies:
            obj = pb.obj
            if hasattr(obj, "x") and hasattr(obj, "y"):
                pb.body.position = (getattr(obj, "x"), getattr(obj, "y"))
            if hasattr(obj, "angle"):
                pb.body.angle = math.radians(getattr(obj, "angle"))
            sx = getattr(obj, "scale_x", 1.0)
            sy = getattr(obj, "scale_y", 1.0)
            size = (pb.base_size[0] * sx, pb.base_size[1] * sy)
            if size != pb.size:
                shapes = list(pb.body.shapes)
                if shapes:
                    self.space.remove(*shapes)
                pb.shape = pymunk.Poly.create_box(pb.body, size)
                self.space.add(pb.shape)
                pb.size = size

        self.space.step(dt)

        for pb in self.bodies:
            obj = pb.obj
            pos = pb.body.position
            if hasattr(obj, "x"):
                obj.x = float(pos.x)
            if hasattr(obj, "y"):
                obj.y = float(pos.y)
            if hasattr(obj, "angle"):
                obj.angle = math.degrees(pb.body.angle)

    def debug_draw(self, renderer) -> None:
        """Draw simple box gizmos for each body using ``renderer``."""
        if pymunk is None:
            raise ImportError("pymunk is required for physics")
        from .gizmos import polyline_gizmo
        import math
        for pb in self.bodies:
            w, h = pb.size
            cx = float(pb.body.position.x)
            cy = float(pb.body.position.y)
            ang = float(pb.body.angle)
            cos_a = math.cos(ang)
            sin_a = math.sin(ang)
            hw = w / 2
            hh = h / 2
            corners = [
                (-hw, -hh),
                (hw, -hh),
                (hw, hh),
                (-hw, hh),
                (-hw, -hh),
            ]
            verts = [
                (
                    cx + x * cos_a - y * sin_a,
                    cy + x * sin_a + y * cos_a,
                )
                for x, y in corners
            ]
            renderer.add_gizmo(
                polyline_gizmo(verts, color=(0.0, 1.0, 0.0, 0.5), thickness=1.0)
            )


class PhysicsExtension(EngineExtension):
    """Engine extension that steps a :class:`PhysicsWorld`."""

    def __init__(self, world: PhysicsWorld | None = None) -> None:
        self.world = world or PhysicsWorld()

    def update(self, engine, dt: float) -> None:  # type: ignore[override]
        self.world.step(dt)
        if hasattr(engine, "renderer") and hasattr(engine.renderer, "add_gizmo"):
            try:
                self.world.debug_draw(engine.renderer)
            except Exception:
                pass
