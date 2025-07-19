"""FrameSync subsystem implementing basic FPS regulation."""
from __future__ import annotations

import time

from sage.config import load_framesync_config

_cfg: dict | None = None
_last: float = 0.0
_target_dt: float = 1.0 / 60.0
_enabled: bool = True
_allow_drift: bool = False
_profile: str = "balanced"
_frame_time: float = 0.0


def boot() -> None:
    """Initialise FrameSync using configuration values."""
    global _cfg, _last, _target_dt, _enabled, _allow_drift, _profile
    _cfg = load_framesync_config()
    _enabled = _cfg.get("enabled", True)
    _target_dt = 1.0 / _cfg.get("target_fps", 60)
    _allow_drift = _cfg.get("allow_drift", False)
    _profile = _cfg.get("profile", "balanced")
    _last = time.perf_counter()


def reset() -> None:
    global _last, _frame_time
    _last = time.perf_counter()
    _frame_time = 0.0


def destroy() -> None:
    reset()


def regulate() -> None:
    """Delay the current thread to maintain the target frame rate."""
    global _last, _frame_time
    if not _enabled:
        _frame_time = 0.0
        _last = time.perf_counter()
        return
    start = time.perf_counter()
    elapsed = start - _last
    remain = _target_dt - elapsed
    if remain > 0:
        sleep_part = remain * 0.9
        if sleep_part > 0:
            time.sleep(sleep_part)
        while (time.perf_counter() - _last) < _target_dt:
            pass
    end = time.perf_counter()
    _frame_time = end - _last
    if _allow_drift:
        _last = end
    else:
        _last += _target_dt


def get_last_frame_time() -> float:
    return _frame_time


def get_actual_fps() -> float:
    if _frame_time <= 0:
        return 0.0
    return 1.0 / _frame_time


__all__ = [
    "boot",
    "reset",
    "destroy",
    "regulate",
    "get_last_frame_time",
    "get_actual_fps",
]
