"""Example built-in roles used in tests."""

from __future__ import annotations

from dataclasses import dataclass
from ..interfaces import Role
from .. import register


@dataclass
class Sprite(Role):
    image: str | None = None

    def on_render(self, ctx) -> None:
        ctx.append(f"render:{self.image}")  # for tests


@dataclass
class PhysicsBody(Role):
    mass: float = 1.0
    updated: bool = False

    def on_update(self, delta: float) -> None:
        self.updated = True


@dataclass
class EnemyAI(Role):
    difficulty: str = "normal"
    called: bool = False

    def on_update(self, delta: float) -> None:
        self.called = True


# register roles
register("Sprite", Sprite)
register("PhysicsBody", PhysicsBody)
register("EnemyAI", EnemyAI)
