"""Time management module."""

from __future__ import annotations

from dataclasses import dataclass
from time import perf_counter

@dataclass
class TimeState:
    dt: float = 0.0
    unscaled_dt: float = 0.0
    time: float = 0.0
    frame: int = 0
    _last: float = perf_counter()
    scale: float = 1.0


def boot(_config: dict) -> None:
    global state
    state = TimeState()


def update() -> None:
    now = perf_counter()
    raw_dt = now - state._last
    state._last = now
    state.unscaled_dt = raw_dt
    state.dt = raw_dt * state.scale
    state.time += state.dt
    state.frame += 1


def reset() -> None:
    state.__dict__.update(TimeState().__dict__)


def get_time() -> TimeState:
    return state
