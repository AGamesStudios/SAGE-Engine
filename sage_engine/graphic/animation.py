"""Property animation helpers for GUI widgets."""
from __future__ import annotations

from ..scheduler import timers


def animate(obj, prop: str, start: float, end: float, duration: int, easing: str = "linear") -> None:
    steps = max(int(duration / 16), 1)
    delta = end - start
    setattr(obj, prop, start)
    counter = {"i": 0}

    def _step():
        t = counter["i"] / steps
        if easing == "ease-in":
            t = t * t
        elif easing == "ease-out":
            t = 1 - (1 - t) * (1 - t)
        value = start + delta * t
        setattr(obj, prop, value)
        counter["i"] += 1
        if counter["i"] > steps:
            setattr(obj, prop, end)
        else:
            timers.manager.set(0.0, _step)

    timers.manager.set(0.0, _step)
