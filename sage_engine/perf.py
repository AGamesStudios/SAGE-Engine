"""Performance detection and scaling helpers."""
from __future__ import annotations

import os
import sys
import resource

_low_perf = False
_scale = 1.0


def detect_low_perf() -> bool:
    """Auto-detect a low performance environment."""
    global _low_perf
    if _low_perf:
        return True
    if "--low-perf" in sys.argv or os.environ.get("SAGE_LOW_PERF") == "1":
        _low_perf = True
    elif (os.cpu_count() or 0) <= 2:
        _low_perf = True
    return _low_perf


def set_low_perf(enabled: bool) -> None:
    global _low_perf
    _low_perf = enabled


def is_low_perf() -> bool:
    return _low_perf


def check_memory(limit_mb: int = 512) -> bool:
    """Return True if memory usage exceeds limit in megabytes."""
    usage_kb = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    if usage_kb == 0:
        return False
    return usage_kb / 1024 > limit_mb


def update_scale(fps: float) -> None:
    """Simple dynamic scaling based on FPS."""
    global _scale
    if fps < 30:
        _scale = max(0.5, _scale - 0.1)
    elif fps > 55:
        _scale = min(1.0, _scale + 0.1)


def get_scale() -> float:
    return _scale

__all__ = [
    "detect_low_perf",
    "set_low_perf",
    "is_low_perf",
    "check_memory",
    "update_scale",
    "get_scale",
]
