"""Performance detection and scaling helpers."""
from __future__ import annotations

import os
import sys
try:
    import psutil  # type: ignore
except Exception:  # module may not be installed
    psutil = None  # type: ignore
try:
    import resource  # type: ignore
except Exception:
    resource = None  # type: ignore

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


def _memory_usage_kb() -> int:
    if psutil is not None:
        try:
            return int(psutil.Process().memory_info().rss / 1024)
        except Exception:
            return 0
    if resource is not None:
        usage = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
        # macOS returns bytes
        if sys.platform == "darwin":
            usage //= 1024
        return usage
    return 0


def check_memory(limit_mb: int = 512) -> bool:
    """Return True if memory usage exceeds limit in megabytes."""
    usage_kb = _memory_usage_kb()
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
