from __future__ import annotations

from dataclasses import dataclass
import random
from typing import List, Tuple

MAX_PARTICLES = 2048

@dataclass
class Particle:
    x: float
    y: float
    vx: float
    vy: float
    life: float
    age: float = 0.0

    @property
    def alpha(self) -> float:
        return max(0.0, 1.0 - self.age / self.life)


class InkEmitter:
    """Simple particle emitter updated on the CPU."""

    def __init__(
        self,
        position: Tuple[float, float] = (0.0, 0.0),
        *,
        rate: float = 10.0,
        velocity_range: Tuple[float, float] = (-1.0, 1.0),
        life_time: float = 1.0,
    ) -> None:
        self.x, self.y = position
        self.rate = rate
        self.velocity_range = velocity_range
        self.life_time = life_time
        self.particles: List[Particle] = []
        self._emit_accum = 0.0

    def emit(self, dt: float) -> None:
        """Spawn particles over *dt* seconds respecting :data:`MAX_PARTICLES`."""
        self._emit_accum += self.rate * dt
        count = int(self._emit_accum)
        self._emit_accum -= count
        for _ in range(count):
            if len(self.particles) >= MAX_PARTICLES:
                break
            vx = random.uniform(*self.velocity_range)
            vy = random.uniform(*self.velocity_range)
            self.particles.append(
                Particle(self.x, self.y, vx, vy, self.life_time)
            )

    def update(self, dt: float) -> None:
        """Advance particle simulation by *dt* seconds."""
        alive: List[Particle] = []
        for p in self.particles:
            p.age += dt
            if p.age < p.life:
                p.x += p.vx * dt
                p.y += p.vy * dt
                alive.append(p)
        self.particles = alive

    def draw(self) -> List[Tuple[float, float, float]]:
        """Return particle positions and alpha values for rendering."""
        return [(p.x, p.y, p.alpha) for p in self.particles]

__all__ = ["InkEmitter", "Particle", "MAX_PARTICLES"]
