"""Property animation helpers for GUI widgets."""
from __future__ import annotations

from ..scheduler import timers
from ..logger import logger


def animate(
    obj,
    prop: str,
    start: float,
    end: float,
    duration: int,
    easing: str = "linear",
    on_finish=None,
):
    """Animate ``prop`` from ``start`` to ``end`` on ``obj`` over ``duration`` ms."""
    if not hasattr(obj, prop):
        logger.error("animation: missing property %s", prop)
        return lambda: None
    try:
        float(start)
        float(end)
        duration = int(duration)
    except Exception as exc:  # pragma: no cover - defensive
        logger.error("animation params invalid: %s", exc)
        return lambda: None

    steps = max(int(duration / 16), 1)
    delta = end - start
    setattr(obj, prop, start)
    counter = {"i": 0}
    cancelled = {"flag": False}

    def _step():
        if cancelled["flag"]:
            return
        t = counter["i"] / steps
        if easing == "ease-in":
            t = t * t
        elif easing == "ease-out":
            t = 1 - (1 - t) * (1 - t)
        elif easing == "ease-in-out":
            t = t * t * (3 - 2 * t)
        value = start + delta * t
        setattr(obj, prop, value)
        counter["i"] += 1
        if counter["i"] > steps:
            setattr(obj, prop, end)
            if on_finish:
                try:
                    on_finish()
                except Exception as e:  # pragma: no cover - user callback
                    logger.error("animation finish error: %s", e)
        else:
            timers.manager.set(0.0, _step)

    timers.manager.set(0.0, _step)

    def cancel():
        cancelled["flag"] = True

    return cancel
